#include "MiniSQLBufferManager.h"
#include "MiniSQLException.h"

BufferManager::Page::Page() {
    filename = "";
    block_id = -1;//文件里是第0块开始
    dirty = false;
    pin = false;
    ref = false;
    empty = true;
    memset(buffer, 0, sizeof(char)*PAGESIZE);
}

//构造函数(初始化页数组)
BufferManager::BufferManager(int page_num) {
    frame = new Page[page_num];
    this->page_num = page_num;
    replace_position = 0;
}

//析构函数:缓冲区全部写回磁盘
BufferManager::~BufferManager() {
    for (int i = 0; i < page_num; i++) {//每一页，写回每一块
        if(frame[i].dirty) writeBackToDisk(i, frame[i].filename, frame[i].block_id);
    }
}

//获取文件中块对应在内存里的页号(没找到就调用其他函数分配一页)
int BufferManager::getPageID(const string &filename, int block_id) {
    /*for (int i = 0; i < page_num; i++) {
        if (frame[i].filename == filename && frame[i].block_id == block_id)
            return i;
    }*/
    auto id = nameID.find(make_pair(filename,block_id));
    if (nameID.end() != id) return (*id).second;
    //出了循环，说明buffer里没有匹配的页
    int page_id = getEmptyPage();
    loadBlockToPage(page_id, filename, block_id);
    return page_id;
}

//读取某页的内容（直接使用文件名）
char* BufferManager::getBlockContent(const string &filename, int block_id) {
    int page_id = getPageID(filename, block_id);
    char* head = frame[page_id].buffer;
    frame[page_id].ref = true;
    return head;
}

//读取某页的内容（使用页号）
char* BufferManager::getBlockContent(int page_id) {
    char* head = frame[page_id].buffer;
    frame[page_id].ref = true;
    return head;
}

//修改某页的内容（直接使用文件名）
void BufferManager::setBlockContent(const string &filename, int block_id, int offset, char* data, size_t length) {
    int page_id = getPageID(filename, block_id);
    if (frame[page_id].empty == true) throw MiniSQLException("Empty Page!");
    if (offset >= PAGESIZE) throw MiniSQLException("Write Page Out of range!");
    strncpy_s(frame[page_id].buffer, data, length);
    frame[page_id].dirty = true;
    frame[page_id].ref = true;
}

//修改某页的内容（使用页号）
void BufferManager::setBlockContent(int page_id, int offset, char* data, size_t length) {
    if (frame[page_id].empty == true) throw MiniSQLException("Empty Page!");
    if (offset >= PAGESIZE) throw MiniSQLException("Write Page Out of range!");
    strncpy_s(frame[page_id].buffer, data, length);
    frame[page_id].dirty = true;
    frame[page_id].ref = true;
}

//钉住/解钉
void BufferManager::setPagePin(int page_id, bool pin) {
    frame[page_id].pin = pin;
}

//时钟替换策略找一个空闲/可替换的页,返回page_id
int BufferManager::getEmptyPage() {
    for (int i = 0; i < page_num; i++) {
        if (frame[i].empty == true) return i;
    }
    //没有空的，采用时钟替换策略
    while (1) {
        if (frame[replace_position].ref == true)
            frame[replace_position].ref = false;
        else if (frame[replace_position].pin == false) {//没被钉住
            if (frame[replace_position].dirty == true) {
                //写回
                string filename = frame[replace_position].filename;
                int block_id = frame[replace_position].block_id;
                writeBackToDisk(replace_position, filename, block_id);
                //清空该页数据（重新初始化）
                frame[replace_position].filename = "";
                frame[replace_position].block_id = -1;
                frame[replace_position].dirty = false;
                frame[replace_position].pin = false;
                frame[replace_position].ref = false;
                frame[replace_position].empty = true;
                memset(frame[replace_position].buffer, 0, sizeof(char) * PAGESIZE);
            }
            break;
        }
        replace_position = (replace_position + 1) % page_num;
    }
    return replace_position;
}
//将文件中的块加载到内存的一页里
void BufferManager::loadBlockToPage(int page_id, const string &filename, int block_id) {
    FILE* fp;
    fopen_s(&fp, filename.c_str(), "r");
    if (fp == nullptr) throw MiniSQLException("Fail to open file!"); //打开文件失败

    //定位和读取
    fseek(fp, sizeof(char) * PAGESIZE * block_id, SEEK_SET);
    char* head = frame[page_id].buffer;
    fread(head, sizeof(char), PAGESIZE, fp);
    fclose(fp);
    frame[page_id].filename = filename;
    frame[page_id].block_id = block_id;
    frame[page_id].dirty = false;
    frame[page_id].pin = false;
    frame[page_id].ref = true;
    frame[page_id].empty = false;
    nameID[make_pair(filename,block_id)] = page_id;
}
//将页写回磁盘
void BufferManager::writeBackToDisk(int page_id, const string &filename, int block_id) {
    FILE* fp;
    fopen_s(&fp, filename.c_str(), "r+");
    if (fp == nullptr) throw MiniSQLException("Fail to open file!"); //打开文件失败

    //定位和写入
    fseek(fp, sizeof(char) * PAGESIZE * block_id, SEEK_SET);
    char* head = frame[page_id].buffer;
    fwrite(head, sizeof(char), PAGESIZE, fp);
    fclose(fp);
}

void BufferManager_test() {
    BufferManager BM = BufferManager();
    try {
        char *head = BM.getBlockContent("../test.txt", 0);
        head = BM.getBlockContent("../test.txt", 1);
        std::cout << head;
        char mod[] = "abcd";
        BM.setBlockContent("../test.txt", 1, 0, mod, sizeof(mod));
        head = BM.getBlockContent("../test.txt", 2);
    } catch (MiniSQLException &e){
        std::cout << e.getMessage();
    }
}