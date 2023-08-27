#ifndef SKIPLIST_H
#define SKIPLIST_H
#include <iostream>
#include <mutex>
#include <fstream>
#include <memory>
#include <cstdlib>
#include <string>
#include <arpa/inet.h>
#include <cstring>

static std::mutex locker;
static std::string delimiter = ":";

#define STORE_FILE "store/DumpFile"
// 节点类
template<typename K, typename V>
class Node {

public:
    // 无参构造
    Node() {}
    // 有参构造， 最后的int为层数
    Node(K k, V v, int);
    // 析构函数
    ~Node();
    // 获取当前节点的Key值
    K get_key() const;
    // 获取当前节点的Value值
    V get_value() const;
    // 设置当前节点的value值
    void set_value(V);
    // forward存储当前节点在第i层的下一个节点
    Node<K, V> **forward;
    // 当前节点所在层级
    int node_level;
private:
    K key;
    V value;
};

// 跳表类
template<typename K, typename V>
class SkipList{

public:
    static SkipList<K, V>* getSkipList (int n);
    // 生成随机层数
    int get_random_level();
    // 创建节点,最后参数为所在的层数
    Node<K, V>* create_node(K, V, int);
    // 插入数据
    std::string insert_element(K ,V);
    // 打印数据，每一层都打印
    std::string display_list();
    // 删除数据
    std::string delete_element(K);
    // 查找数据
    std::string search_element(K);
    // 存入文件
    std::string dump_file();
    // 加载文件
    std::string load_file();
    // 返回节点个数
    std::string size();
    // 初始化

private:

    SkipList() {};
    // 有参构造，int值为最大层数
    SkipList(int);
    // 析构函数
    ~SkipList();

    // 从文件中的一行获取Key，Value
    void get_key_value_by_string(const std::string& str, std::string* key, std::string* value);
    // 是否为有效的string
    bool is_valid_string(const std::string& str);

private:
    // 该跳表最大层数
    int max_level;
    // 该跳表当前层数
    int cur_level;
    // 该跳表当前元素数
    int element_count;

    // 当前跳表头节点
    Node<K, V>* header;

    //文件操作
    std::ifstream file_reader;
    std::ofstream file_writer;
};

template<typename K, typename V>
Node<K, V>::Node(const K k, const V v, int level) : key(k), value(v), node_level(level) {
    // level + 1，因为数组索引是从 0 到 level
    this->forward = new Node<K, V>*[level+1];

    // 使用 nullptr 来填充 forward 数组
    std::fill_n(forward, level+1, nullptr);
}

template<typename K, typename V>
Node<K, V>::~Node() {
    delete[] forward;
}

template<typename K, typename V>
K Node<K, V>::get_key() const {
    return key;
}

template<typename K, typename V>
V Node<K, V>::get_value() const {
    return value;
}

template<typename K, typename V>
void Node<K, V>::set_value(V v) {
    this->value = v;
}

// 跳表类
template<typename K, typename V>
SkipList<K, V>* SkipList<K, V>::getSkipList (int n) {
    static SkipList<K, V> list(n);
    return &list;
}

template<typename K, typename V>
Node<K, V>* SkipList<K, V>::create_node(const K k, const V v, int level) {
    Node<K, V>* res = new Node<K, V>(k, v, level);
    return res;
}

template<typename K, typename V>
std::string SkipList<K, V>::insert_element(const K key, const V value) {
    std::string res;
    locker.lock();
    Node<K, V> *current = this->header;

    // 初始化updata数组，用于存储待修改节点
    Node<K, V>* update[max_level+1] = {nullptr};

    // 从跳表最高层开始，每一层从头节点开始遍历
    for(int i = cur_level; i >=0; --i) {

        // 如果当前节点的下一个节点的key大于当前节点或下一个节点不存在，则保存当前节点位置
        while(current->forward[i] != nullptr && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }

    current = current->forward[0];

    // 当前插入节点已存在，返回1
    if (current != NULL && current->get_key() == key) {
        res = "key: " + std::to_string(key) + ", exists\n";
        locker.unlock();
        return res;
    }

    // 更新当前跳表的层数
    int random_level = get_random_level();
    if(random_level > cur_level) {

        // 在新增层将updata设置为头节点
        for(int i = cur_level+1; i < random_level+1; ++i) {
            update[i] = header;
        }
        cur_level = random_level;
    }

    Node<K, V>* new_node = new Node<K, V>(key, value, random_level);

    // 遍历插入
    for(int i = 0; i < random_level+1; ++i){
        new_node->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = new_node;
    }

    res += "Successfully inserted key:" + std::to_string(key) + ", value:" + value + "\n";
    ++element_count;

    locker.unlock();
    return res;
}

template<typename K, typename V>
std::string SkipList<K, V>::display_list() {
    std::string res;
    res += "\n----------Skip List----------\n";
    for(int i = 0; i < cur_level; ++i) {
        Node<K, V>* node = this->header->forward[i];
        res += "Level " + std::to_string(i) + ": ";
        while(node != nullptr) {
            res += std::to_string(node->get_key()) + " : " + node->get_value() + " ; ";
            node = node->forward[i];
        }
        res += "\n";
    }
    return res;
}

template<typename K, typename V>
std::string SkipList<K, V>::dump_file() {
    std::string s;
    s += "----------Dump File----------\n";
    file_writer.open(STORE_FILE);
    if (!file_writer.is_open()) {
        // 文件打开失败的处理逻辑
        std::cout << "Failed to open file " << STORE_FILE << " with error: " << std::strerror(errno) << std::endl;
    }
    Node<K, V>* node = this->header->forward[0];

    while(node != nullptr) {
        file_writer << node->get_key() << " : " << node->get_value() << "\n"; 
        s += std::to_string(node->get_key()) + " : " + node->get_value() + "\n"; 
        node = node->forward[0];
    }

    file_writer.flush();
    file_writer.close();
    return s;
}

template<typename K, typename V>
std::string SkipList<K, V>::load_file() {

    std::string s;
    s += "----------Load File----------\n";
    file_reader.open(STORE_FILE);
    std::string line;
    std::string* key = new std::string();
    std::string* value = new std::string();
    while(getline(file_reader, line)) {
        get_key_value_by_string(line, key, value);
        if(key == nullptr || value == nullptr) {
            continue;
        }
        insert_element(stoi(*key), *value);
        s += "key: " + *key + "value: " + *value + "\n";
    }
    delete key;
    delete value;
    file_reader.close();
    return s;
}

template<typename K, typename V>
std::string SkipList<K, V>::size() {
    return "The SkipList' size is " + std::to_string(element_count);
}

template<typename K, typename V>
void SkipList<K, V>::get_key_value_by_string(const std::string& str, std::string* key, std::string* value) {

    if (!is_valid_string(str)) {
        return;
    }
    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter)+2, str.length());
}

template<typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string& str) {
    if (str.empty()) {
        return false;
    }
    if (str.find(delimiter) == std::string::npos) {
        return false;
    }
    return true;
}

// 遍历跳表以找到具有给定键的节点的位置
template<typename K, typename V>
std::string SkipList<K, V>::delete_element(K key) {
    std::string s;
    locker.lock();
    Node<K, V> *current = this->header;
    Node<K, V>* update[max_level+1] = {nullptr};

    for(int i = cur_level; i >= 0; --i) {
        while(current->forward[i] != nullptr && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }

        // 在每层存储上一个节点
        update[i] = current;
    }

    current = current->forward[0];
    if(current != nullptr && current->get_key() == key) {

        // 更新前置节点的指针以删除当前节点
        for(int i = 0; i <= cur_level; ++i) {
            if(update[i]->forward[i] != current) {
                break;
            }

            update[i]->forward[i] = current->forward[i];
        }

        // 如果当前层变为空，则降低当前层数
        while(cur_level > 0 && header->forward[cur_level] == 0) {
            --cur_level;
        }

        delete current;

        s += "Successfully deleted key " + std::to_string(key) + "\n";
        --element_count;
        locker.unlock();
        return s; 
    }
    else {
        locker.unlock();
        return s; 
    }
}

template<typename K, typename V>
std::string SkipList<K, V>::search_element(K key) {
    std::string s;
    s += "----------search_element----------\n";
    Node<K, V>* current = this->header;

    for(int i = cur_level; i >= 0; --i) {
        while(current->forward[i] != nullptr && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
    }

    current = current->forward[0];

    if(current != nullptr && current->get_key() == key) {
        s += "Found key: " + std::to_string(key) + ", value: " + current->get_value() + "\n";
        return s;
    }

    s += "Not Found Key : " + std::to_string(key) + "\n";
    return s;
}

template<typename K, typename V>
SkipList<K, V>::SkipList(int max_level) : max_level(max_level), element_count(0), cur_level(0) {
    K k;
    V v;
    this->header = new Node<K, V>(k, v, max_level);
}

template<typename K, typename V>
SkipList<K, V>::~SkipList() {
    if(file_reader.is_open()) {
        file_reader.close();
    }
    if(file_writer.is_open()) {
        file_writer.close();
    }
    delete header;
}

template<typename K, typename V>
int SkipList<K, V>::get_random_level() {

    int k = 1;
    while(rand() % 2) {
        ++k;
    }
    return k > max_level ? max_level : k;
}


#endif