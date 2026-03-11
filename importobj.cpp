#include "importobj.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

struct Vector3 {
    float x, y, z;
};

static int parseF(const std::string& part)
{
    size_t slashPos = part.find('/');
    if (slashPos == std::string::npos) {
        return std::stoi(part);
    }
    return std::stoi(part.substr(0, slashPos));
}

bool loadFile(const std::string& filename, std::vector<float>& vertices)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Failed to open OBJ file: " << filename << std::endl;
        return false;
    }

    std::vector<Vector3> positions;
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            Vector3 p;
            iss >> p.x >> p.y >> p.z;
            positions.push_back(p);
        }
        else if (prefix == "f") {
            std::vector<int> faceIndices;
            std::string part;

            while (iss >> part) {
                faceIndices.push_back(parseF(part));
            }

            for (size_t i = 1; i + 1 < faceIndices.size(); i++) {
                int i1 = faceIndices[0];
                int i2 = faceIndices[i];
                int i3 = faceIndices[i + 1];

                Vector3 p1 = positions[i1 - 1];
                Vector3 p2 = positions[i2 - 1];
                Vector3 p3 = positions[i3 - 1];

                vertices.push_back(p1.x);
                vertices.push_back(p1.y);
                vertices.push_back(p1.z);

                vertices.push_back(p2.x);
                vertices.push_back(p2.y);
                vertices.push_back(p2.z);

                vertices.push_back(p3.x);
                vertices.push_back(p3.y);
                vertices.push_back(p3.z);
            }
        }
    }

    return true;
}