#pragma once

#include <string>
#include <map>
using std::string;
using std::map;
using std::pair;

#define PAGESIZE 4096   //一页4KB
#define MAXPAGENUM 2 //最多100页

class BufferManager {
private:
    struct Page {
        Page();
        char buffer[PAGESIZE];//块内容
        string filename;//映射文件名
        int block_id;//映射块号
        bool dirty;//修改标记
        bool pin;//锁定标记
        bool ref;//使用标记（时钟替换）
        bool empty;//空标记
    };

    //动态分配页数组
    int page_num;//页数
    Page* frame;//数组首地址指针
    map<pair<string,int>, int> nameID;
    int replace_position;//时钟指针（时钟替换）
public:
    BufferManager(int page_num = MAXPAGENUM);//构造函数(初始化页数组)
    ~BufferManager();//析构函数

    //获取文件中块对应在内存里的页号
    int getPageID(const string &filename, int block_id);

    //读取某页的内容
    char* getBlockContent(const string &filename, int block_id);
    char* getBlockContent(int page_id);

    //修改某页的内容
    void setBlockContent(const string &filename, int block_id, int offset, char* data, size_t length);
    void setBlockContent(int page_id, int offset, char* data, size_t length);

    //在文件中新开一块，返回对应的页号
    int allocNewBlock(const string &filename);

    //清空某文件相关的所有页
    void setEmpty(const string &filename);
    
    //固定/解除固定
    void setPagePin(int page_id, bool pin);

    //时钟替换策略找一个空闲/可替换的页,返回page_id
    int getEmptyPage();

    //将文件中的块加载到内存的一页里
    void loadBlockToPage(int page_id, const string &file_name, int block_id);

    //将页写回磁盘
    void writeBackToDisk(int page_id, const string &file_name, int block_id);
};