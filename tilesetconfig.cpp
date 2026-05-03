#include "tilesetconfig.h"
#include <fstream>
#include <iostream>
#include <map>
#include <functional>
#include <opencv2/core/types.hpp>
#include <opencv2/opencv.hpp>
#include "sides.h"
#include "tile.h"

using std::make_shared;
double computeSSIM(const cv::Mat& img1, const cv::Mat& img2) //Some image comparing magic here, more robust than hashing 
{
    const double C1 = 6.5025, C2 = 58.5225;
    cv::Mat img1_float, img2_float;
    img1.convertTo(img1_float, CV_32F);
    img2.convertTo(img2_float, CV_32F);

    cv::Mat mu1, mu2;
    cv::GaussianBlur(img1_float, mu1, cv::Size(11, 11), 1.5);
    cv::GaussianBlur(img2_float, mu2, cv::Size(11, 11), 1.5);

    cv::Mat mu1_sq = mu1.mul(mu1);
    cv::Mat mu2_sq = mu2.mul(mu2);
    cv::Mat mu1_mu2 = mu1.mul(mu2);

    cv::Mat sigma1_sq, sigma2_sq, sigma12;

    cv::GaussianBlur(img1_float.mul(img1_float), sigma1_sq, cv::Size(11, 11), 1.5);
    sigma1_sq -= mu1_sq;

    cv::GaussianBlur(img2_float.mul(img2_float), sigma2_sq, cv::Size(11, 11), 1.5);
    sigma2_sq -= mu2_sq;

    cv::GaussianBlur(img1_float.mul(img2_float), sigma12, cv::Size(11, 11), 1.5);
    sigma12 -= mu1_mu2;

    cv::Mat ssim_map;
    ssim_map = ((2 * mu1_mu2 + C1) .mul (2 * sigma12 + C2)) / ((mu1_sq + mu2_sq + C1) .mul (sigma1_sq + sigma2_sq + C2));

    cv::Scalar mssim = cv::mean(ssim_map);

    double ssim = (mssim[0] + mssim[1] + mssim[2]) / 3;

    return ssim;
}

vector<cv::Mat> splitImageIntoGrid(cv::Mat& image, int gridSize) {
    vector<cv::Mat> gridImages;

    int subImageWidth = image.cols / gridSize;
    int subImageHeight = image.rows / gridSize;

    for (int y = 0; y < gridSize; ++y) {
        for (int x = 0; x < gridSize; ++x) {
            cv::Rect square(x * subImageWidth, y * subImageHeight, subImageWidth, subImageHeight);

            cv::Mat subImage = image(square);
            gridImages.push_back(subImage);
        }
    }

    return gridImages;
}


TilesetConfig::TilesetConfig(string config_path) : path(config_path)
{
    std::ifstream file(config_path);

    if(file.fail()){
        std::cerr << "Failed to open file." << '\n';
        return;
    }

    file >> config;
}

vector<TileInfo> TilesetConfig::getTilesInfo()
{
    vector<TileInfo> res;
    if(config["type"] == "tile") 
    {
        for(const auto& [key, tile] : config["tiles"].items())
        {
            res.push_back(TileInfo { tile["address"], tile["sides"], vector<vector<int>>(), key, tile["rotate"]});
        }
    }
    else if(config["type"] == "overlap")
    {
        string texPath = path + "/" + (string)config["texture"];
        cv::Mat texture = cv::imread(texPath , cv::IMREAD_COLOR); // Opening texture and extracting info from config
        int gridSize = config["grid_size"];
        int overlapSize = config["overlap_size"];

        std::map<std::string, bool> rule_autofill_options;
        for(json::iterator it = config["rule_autofill_options"].begin(); it != config["rule_autofill_options"].end(); it++) 
        {
            rule_autofill_options[it.key()] = it.value();
        }
        bool repeating = rule_autofill_options["repeating"];
        bool sphere = rule_autofill_options["sphere"];

        vector<cv::Mat> gridImages = splitImageIntoGrid(texture, gridSize); //Creating grid

        double threshold = 0.9;
        std::map<int, cv::Mat> imageMap;
        std::map<int, int> countMap;
        

        for (int i = 0; i < gridImages.size(); i++) // Count how much same tile appears in the texture
        {
            imageMap[i] = gridImages[i];
            for (int j = 0; j < i; ++j)
            {
                double ssim = computeSSIM(gridImages[i], gridImages[j]);
                if (ssim > threshold)
                {
                    countMap[j]++;
                    break;
                }
            }
            if (countMap.find(i) == countMap.end())
            {
                countMap[i] = 1;
            }
        }

        vector<vector<int>> grid(gridSize, vector<int>(gridSize));

        //Creating grid with IDs from tiles
        int i = 0;
        for (auto& row : grid) {
            for (auto& cell : row) {
                auto it = std::next(imageMap.begin(), i % imageMap.size());
                cell = it->first;
                i++;
            }
        }

        //Create patterns
        vector<shared_ptr<Pattern>> patterns;
        int index = 0;
        for (int i = 0; i <= gridSize - overlapSize; i++) {

            for (int j = 0; j <= gridSize - overlapSize; ++j) {
                vector<vector<int>> patternGrid;
                for (int k = i; k < i + overlapSize; ++k) {
                    vector<int> row;
                    for (int l = j; l < j + overlapSize; ++l) {
                        row.push_back(grid[k][l]);
                    }
                    patternGrid.push_back(row);
                }
                if (!patternGrid.empty()) {
                    auto pattern = std::make_shared<Pattern>(patternGrid, index);
                    index++;
                    patterns.push_back(pattern);
                }
            }
        }
        // vector<shared_ptr<Pattern>> newPatterns;

        // for(auto& pattern : patterns) //Create rotations
        // {
        //     for(int i = 1; i < 4; i++)
        //     {
        //         shared_ptr<Pattern> newPattern = make_shared<Pattern>(*pattern);
        //         newPattern->rotateClockwise(i);
        //         newPatterns.push_back(newPattern);
        //     }
        // }

        // patterns.insert(patterns.end(), newPatterns.begin(), newPatterns.end());

        for (const auto& pattern : patterns) {
            std::cout << "Pattern:\n";
            for (const auto& row : pattern->grid) {
                for (const auto& cell : row) {
                    std::cout << cell << ' ';
                }
                std::cout << '\n';
            }
            std::cout << '\n';
        }

        // // Add neighbors
        // std::cout << patterns.size() << "\n";
        // //Generate rules
        // for (auto& pattern : patterns) {
        //     for (int dir : const_dir) {
        //         string checkingSide = pattern->sides.at(dir);
        //         int oppositeSide = rotateSide(dir);
        //         bool found = false;
        //         for (auto& possibleNeighbor : patterns) {
        //             //std::cout << "Pattern " << pattern->ID << " side " << dir << " (" << checkingSide << ") vs. Pattern " << possibleNeighbor->ID << " side " << oppositeSide << " (" << reverseString(possibleNeighbor->sides.at(oppositeSide)) << ")\n";
        //             if (reverseString(possibleNeighbor->sides.at(oppositeSide)) == checkingSide) {
        //                 pattern->addNeighbor(possibleNeighbor->ID, dir);
        //                 //std::cout << "Match found: Pattern " << pattern->ID << " side " << dir << " with Pattern " << possibleNeighbor->ID << " side " << oppositeSide << '\n';
        //                 found = true;
        //             }
        //         }

        //         if (!found) {
        //             string checkingSide = pattern->sides.at(oppositeSide);
        //             for (auto& possibleNeighbor : patterns) {
        //                 //std::cout << "Fallback: Pattern " << pattern->ID << " side " << oppositeSide << " (" << checkingSide << ") vs. Pattern " << possibleNeighbor->ID << " side " << dir << " (" << reverseString(possibleNeighbor->sides.at(dir)) << ")\n";
        //                 if (reverseString(possibleNeighbor->sides.at(oppositeSide)) == checkingSide) {
        //                     pattern->addNeighbor(possibleNeighbor->ID, dir);
        //                     //std::cout << "Fallback match: Pattern " << pattern->ID << " side " << oppositeSide << " with Pattern " << possibleNeighbor->ID << " side " << dir << '\n';
        //                 }
        //             }
        //         }
        //     }
        // }


        // //Generating image, saving it and creating new pattern
        // for (int i = 0; i < patterns.size(); ++i) 
        // {
        //     vector<cv::Mat> rows;
        //     for (int j = 0; j < patterns[i]->grid.size(); ++j) {
        //         vector<cv::Mat> row;
        //         for (int k = 0; k < patterns[i]->grid[j].size(); ++k) {
        //             row.push_back(imageMap[patterns[i]->grid[j][k]]);
        //         }
        //         cv::Mat rowImage;
        //         cv::hconcat(row.data(), row.size(), rowImage);
        //         rows.push_back(rowImage);
        //     }
        //     cv::Mat image;
        //     cv::vconcat(rows.data(), rows.size(), image);
        //     string imagePath = path + "/tile" + std::to_string(i) + ".png";
        //     cv::imwrite(imagePath, image);

        //     //vector<int> 
        //     string ID = std::to_string(i);
        //     res.push_back({imagePath, patterns[i]->sides, patterns[i]->neighbors, ID, true});
        // }





    }

    for(auto t : res)
        std::cout << t << "\n";

    return res;
}

string TilesetConfig::getGenerationType()
{

    return config["type"];

}
