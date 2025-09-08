#include <fasttext/fasttext.h>
#include "cppjieba/Jieba.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

bool isNearZero(const fasttext::Vector& vec, float threshold = 1e-6) {
    float norm = 0.0;
    for (int i = 0; i < vec.size(); ++i) {
        norm += vec[i] * vec[i];

    }
    return (norm < threshold);

}

// 余弦相似度计算函数
float cosineSimilarity(const fasttext::Vector& vec1, const fasttext::Vector& vec2) {
    if (vec1.size() == 0 || vec2.size() == 0 || vec1.size() != vec2.size()) 
        return -1.0f;

    float dot = 0.0, norm1 = 0.0, norm2 = 0.0;
    for (int i = 0; i < vec1.size(); ++i) {
        dot += vec1[i] * vec2[i];
        norm1 += vec1[i] * vec1[i];
        norm2 += vec2[i] * vec2[i];
    }

    if (norm1 < 1e-6 || norm2 < 1e-6) return 0.0f;
    return dot / (sqrt(norm1) * sqrt(norm2));
}



// 获取文本的向量表示（平均词向量）
fasttext::Vector getTextVector(fasttext::FastText& model, const std::string& text, cppjieba::Jieba& jieba) {
    int dim = model.getDimension();
    fasttext::Vector textVec(dim);
    textVec.zero(); // 初始化为零向量
    
    // 中文分词
    std::vector<std::string> words;
    //jieba.Cut(text, words, true);
    jieba.extractor.Extract(text, words, 20);
    std::cout << words << std::endl;
    
    int validWords = 0;
    for (const auto& word : words) {
        fasttext::Vector wordVec(dim);
        model.getWordVector(wordVec, word);
        
        // 累加有效词向量
        if (!isNearZero(wordVec)) {
            textVec.addVector(wordVec);
            validWords++;
        }
    }
    
    // 计算平均值
    if (validWords > 0) {
        textVec.mul(1.0f / validWords);
    }
    return textVec;
}

float maxSimilarity(fasttext::FastText& model, 
                    const fasttext::Vector& queryVec, 
                    const std::vector<std::string>& keys, 
                    cppjieba::Jieba& jieba) {
    float maxSim = -1.0f;

    for (const auto& key : keys) {
        fasttext::Vector keyVec = getTextVector(model, key, jieba);
        float sim = cosineSimilarity(queryVec, keyVec);
        std::cout << "与关键句 '" << key << "' 相似度: " << sim << "\n";

        if (sim > maxSim) {
            maxSim = sim;
        }
    }
    return maxSim;
}
