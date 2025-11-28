#include "obj_loader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <tuple>
#include <algorithm>

ImportedModel::ImportedModel(const char* path)
{
    loadOBJ(path);
}

ImportedModel::~ImportedModel()
{
    if (EBO) glDeleteBuffers(1, &EBO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);
}

struct VertexKey {
    int v;
    int vt;
    int vn;

    bool operator==(const VertexKey& other) const {
        return v == other.v && vt == other.vt && vn == other.vn;
    }
};

struct VertexKeyHash {
    std::size_t operator()(const VertexKey& k) const noexcept {
        std::size_t h1 = std::hash<int>()(k.v);
        std::size_t h2 = std::hash<int>()(k.vt);
        std::size_t h3 = std::hash<int>()(k.vn);
        return ((h1 * 73856093) ^ (h2 * 19349663) ^ (h3 * 83492791));
    }
};

void ImportedModel::loadOBJ(const char* path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "ERRO: Nao foi possivel abrir o arquivo OBJ: " << path << std::endl;
        return;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;

    std::vector<VertexData> finalVertices;
    std::vector<unsigned int> finalIndices;

    std::unordered_map<VertexKey, unsigned int, VertexKeyHash> vertexMap;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#')
            continue;

        std::stringstream ss(line);
        std::string tag;
        ss >> tag;

        if (tag == "v") {
            glm::vec3 p;
            ss >> p.x >> p.y >> p.z;
            positions.push_back(p);
        }
        else if (tag == "vt") {
            glm::vec2 uv;
            ss >> uv.x >> uv.y;
            uvs.push_back(uv);
        }
        else if (tag == "vn") {
            glm::vec3 n;
            ss >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
        else if (tag == "f") {
            std::vector<std::string> faceTokens;
            std::string vertToken;
            while (ss >> vertToken) {
                faceTokens.push_back(vertToken);
            }

            if (faceTokens.size() < 3) {
                continue;
            }

            std::vector<unsigned int> faceIndices; 
            faceIndices.reserve(faceTokens.size());

            auto processVertexToken = [&](const std::string &tok) -> unsigned int {
                int vIndex = 0, tIndex = 0, nIndex = 0;

                int slashCount = std::count(tok.begin(), tok.end(), '/');

                if (slashCount == 0) {
                    vIndex = std::stoi(tok);
                } else {
                    std::stringstream vss(tok);
                    std::string vStr, tStr, nStr;

                    std::getline(vss, vStr, '/');
                    std::getline(vss, tStr, '/');
                    if (!vss.eof()) {
                        std::getline(vss, nStr, '/');
                    }

                    if (!vStr.empty()) vIndex = std::stoi(vStr);
                    if (!tStr.empty()) tIndex = std::stoi(tStr);
                    if (!nStr.empty()) nIndex = std::stoi(nStr);
                }

                auto fixIndex = [](int idx, int size) -> int {
                    if (idx > 0) {
                        return idx - 1;
                    } else if (idx < 0) {
                        return size + idx;
                    }
                    return -1;
                };

                int pv = fixIndex(vIndex, static_cast<int>(positions.size()));
                int pt = fixIndex(tIndex, static_cast<int>(uvs.size()));
                int pn = fixIndex(nIndex, static_cast<int>(normals.size()));

                if (pv < 0 || pv >= (int)positions.size()) {
                    std::cerr << "WARN: indice de posicao fora do range: " << vIndex 
                              << " em linha: " << line << std::endl;
                    pv = -1;
                }
                if (pt != -1 && (pt < 0 || pt >= (int)uvs.size())) {
                    std::cerr << "WARN: indice de UV fora do range: " << tIndex 
                              << " em linha: " << line << std::endl;
                    pt = -1;
                }
                if (pn != -1 && (pn < 0 || pn >= (int)normals.size())) {
                    std::cerr << "WARN: indice de normal fora do range: " << nIndex 
                              << " em linha: " << line << std::endl;
                    pn = -1;
                }

                VertexKey key{ pv, pt, pn };

                auto it = vertexMap.find(key);
                if (it != vertexMap.end()) {
                    return it->second;
                }

                VertexData vd{};
                if (pv != -1) vd.position = positions[pv];
                if (pt != -1) vd.uv       = uvs[pt];
                if (pn != -1) vd.normal   = normals[pn];

                unsigned int newIndex = (unsigned int)finalVertices.size();
                finalVertices.push_back(vd);
                vertexMap[key] = newIndex;

                return newIndex;
            };

            for (const auto& tok : faceTokens) {
                unsigned int idx = processVertexToken(tok);
                faceIndices.push_back(idx);
            }

            for (size_t i = 1; i + 1 < faceIndices.size(); ++i) {
                finalIndices.push_back(faceIndices[0]);
                finalIndices.push_back(faceIndices[i]);
                finalIndices.push_back(faceIndices[i + 1]);
            }
        }
    }

    file.close();

    if (finalVertices.empty() || finalIndices.empty()) {
        std::cerr << "ERRO: modelo vazio apos carregar OBJ: " << path << std::endl;
        return;
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 finalVertices.size() * sizeof(VertexData),
                 finalVertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 finalIndices.size() * sizeof(unsigned int),
                 finalIndices.data(),
                 GL_STATIC_DRAW);

    // layout: position (0), uv (1), normal (2)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        sizeof(VertexData),
        (void*)offsetof(VertexData, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE,
        sizeof(VertexData),
        (void*)offsetof(VertexData, normal));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(
        3, 2, GL_FLOAT, GL_FALSE,
        sizeof(VertexData),
        (void*)offsetof(VertexData, uv));

    glBindVertexArray(0);

    if (finalIndices.empty() || finalVertices.empty()) {
    std::cerr << "AVISO: Modelo carregado, mas sem vertices ou indices validos. Nao foi possivel desenhar." << std::endl;
    return;
}
    indexCount = (GLsizei)finalIndices.size();

    std::cout << "Modelo carregado: " << path 
              << " (vertices: " << finalVertices.size()
              << ", indices: " << indexCount << ")" << std::endl;
}

void ImportedModel::Draw() const
{
    static bool first = true;
    if (first) {
        std::cout << "[ImportedModel::Draw] VAO=" << VAO
                  << " indexCount=" << indexCount << std::endl;
        first = false;
    }

    if (VAO == 0 || indexCount == 0) {
        std::cout << "[ImportedModel::Draw] Nada a desenhar" << std::endl;
        return;
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}
