#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bits/stdc++.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <queue>
#include <stack>


using namespace std;
// --------------------------------------------------------------------------------
// split_string
void split_string(std::vector<std::string>& ret, std::string s, std::string delimiter)
{
    size_t pos = 0;
    std::string token;
    while((pos = s.find(delimiter)) != std::string::npos)
    {
        token = s.substr(0, pos);
        ret.push_back(token);
        s.erase(0, pos + delimiter.length());
    }

    ret.push_back(s); // push back final one
}

// split_string2
vector<string> split_string2(string input, char delimiter)
{
    vector<string> strings;

    istringstream f(input);
    string s;
    while(getline(f, s, delimiter))
    {
        // cout << s << endl;
        strings.push_back(s);
    }

    return strings;
}

// show_string_vector
void show_string_vector(std::vector<std::string> str_vector, string delimiter)
{
    int i;
    for(i = 0; i < str_vector.size(); i++)
        cout << str_vector[i] << delimiter;
    cout << endl;
}

// --------------------------------------------------------------------------------
// clear_queue
void clear_queue(std::queue<string>& q)
{
    std::queue<string> empty;
    std::swap(q, empty);
}

// print_queue
void print_queue(std::string prompt, std::queue<string> q) // copy a queue inside, because destroy inside
{
    cout << prompt << ": ";

    while(!q.empty())
    {
        std::cout << q.front() << " ";
        q.pop();
    }
    std::cout << std::endl;
}
// --------------------------------------------------------------------------------
// trim from start (in place)
static inline void ltrim(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
}

// trim from end (in place)
static inline void rtrim(std::string& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string& s)
{
    ltrim(s);
    rtrim(s);
}

// ts_code,trade_date,open,high,low,close,pre_close,change,pct_chg,vol,amount
class Stock
{
public:
    char _ts_code[16];
    char _trade_date[16];
    double _open;
    double _high;
    double _low;
    double _close;
    double _pre_close;
    double _change;
    double _pct_chg;
    double _vol;
    double _amount;

    Stock()
    {
        strcpy(_ts_code, "");
        strcpy(_trade_date, "");

        _open = 0.0;
        _high = 0.0;
        _low = 0.0;
        _close = 0.0;

        _pre_close = 0.0;
        _change = 0.0;
        _pct_chg = 0.0;
        _vol = 0.0;

        _amount = 0.0;
    }
    Stock(string _line) // s001,li gang,99,88,99,286
    {
        string stock_line = _line;
        string delimiter = ",";

        std::vector<std::string> ret;
        split_string(ret, stock_line, delimiter);

        strcpy(_ts_code, ret[0].c_str());
        strcpy(_trade_date, ret[1].c_str());

        _open = atof(ret[2].c_str());
        _high = atof(ret[3].c_str());
        _low = atof(ret[4].c_str());
        _close = atof(ret[5].c_str());

        _pre_close = atof(ret[6].c_str());
        _change = atof(ret[7].c_str());
        _pct_chg = atof(ret[8].c_str());
        _vol = atof(ret[9].c_str());

        _amount = atof(ret[10].c_str());
    }

    string toString()
    {
        ostringstream os;
        os << _ts_code << ",";
        os << _trade_date << ",";

        os << _open << ",";
        os << _high << ",";
        os << _low << ",";
        os << _close << ",";

        os << _pre_close << ",";
        os << _change << ",";
        os << _pct_chg << ",";
        os << _vol << ",";

        os << _amount;

        return os.str();
    }
};
// lessThan_Stock
bool lessThan_Stock(Stock& lhs, Stock& rhs)
{
    if(strcmp(lhs._ts_code, rhs._ts_code) < 0)
    {
        return true;
    }
    else if(strcmp(lhs._ts_code,rhs._ts_code) == 0)
    {
        if(strcmp(lhs._trade_date,rhs._trade_date) < 0)
        {
            return true;
        }
        else if(strcmp(lhs._trade_date, rhs._trade_date) == 0)
        {
            return true;
        }
        else if(strcmp(lhs._trade_date, rhs._trade_date) > 0)
        {
            return false;
        }
    }
    else if(strcmp(lhs._ts_code, rhs._ts_code) > 0)
    {
        return false;
    }
}

// init_runs_stock
std::vector<string> init_runs_stock(string file_input, int run_size)
{
    std::vector<string> ret_file_list; // f1 f2 f3 ...

    Stock* p = new Stock[run_size];

    int file_no = 1;
    int i;
    ifstream fin(file_input.c_str());

    while(1)
    {
        int count_this_run = 0;
        for(i = 0; i < run_size; i++)
        {
            char temp_line[256 + 1];

            string line1 = "";
            temp_line[0] = '\0';
            fin.getline(temp_line, 256);
            line1 = temp_line;
            trim(line1);
            bool status1 = true;
            if(line1 == "")
                status1 = false;

            if(status1 == false)
                break;
            count_this_run++;
            p[i] = Stock(line1); //构造函数 line1
        }

        if(count_this_run > 0)
        {
            string file_name_out = "f";
            stringstream ss;
            ss << file_no;
            string strtemp = ss.str();
            file_name_out = file_name_out + strtemp; //构造文件名f1,f2,f3,...
            file_no++;

            cout << "init_runs_stock sort to file" << file_name_out << " rows " << count_this_run << endl;

            sort(p, p + count_this_run, lessThan_Stock); // count_this_run这么多个Student

            ofstream fout(file_name_out.c_str());
            for(i = 0; i < count_this_run; i++)
            {
                fout << p[i].toString() << endl;
            };
            fout.close();

            ret_file_list.push_back(file_name_out);
        }
        else
        {
            break;
        }
    }

    fin.close();
    delete[] p;

    return ret_file_list;
}
void merge_sort_file(string file_name_1, string file_name_2, string file_name_target)
{
    ifstream file1(file_name_1);
    ifstream file2(file_name_2);
    ofstream target(file_name_target);

    string line1,line2;

    getline(file1,line1);
    getline(file2,line2);

    while(!file1.eof() && !file2.eof()){
        Stock stock1(line1);
        Stock stock2(line2);

        if(lessThan_Stock(stock1,stock2)){
            target<<line1<<endl;
            getline(file1,line1);
        }else{
            target<<line2<<endl;
            getline(file2,line2);
        }
    }

    while(!file1.eof()){
        target<<line1<<endl;
        getline(file1,line1);
    }
    while(!file2.eof()){
        target<<line2<<endl;
        getline(file2,line2);
    }
    file1.close();
    file2.close();
    target.close();
}

//用时间进行命名，否则文件名过长会无法生成
string generate_short_filename(int index){
    stringstream ss;
    auto now = chrono::system_clock::now();
    auto now_ms = chrono::time_point_cast<chrono::milliseconds>(now);
    auto epoch = now_ms.time_since_epoch();
    auto value = chrono::duration_cast<chrono::milliseconds>(epoch);

    ss<<"file_"<<setfill('0')<<value.count()<<"_"<<index<<".txt";
    return ss.str();
}

string multi_round_2_way_merge_sort(string file_input, int RUN_SIZE) {
    // 置换阶段
    vector<string> file_list = init_runs_stock(file_input, RUN_SIZE);

    queue<string> source_queue;
    for (int i = 0; i < file_list.size(); i++)
        source_queue.push(file_list[i]);
    print_queue("file_list", source_queue);
    cout << endl;

    // 多趟2way合并
    queue<string> dest_queue;
    string file_name_final = "";
    bool done = false;
    int round = 1;
    while (!done) {
        cout << "round " << round << endl;
        print_queue("source_queue", source_queue);

        if (source_queue.size() == 1) { // only one file in queue
            file_name_final = source_queue.front();
            done = true;
            cout << "done" << endl;
            break;
        }

        while (!source_queue.empty()) {
            string file_name_1 = source_queue.front(); // fetch one file
            source_queue.pop();
            if (source_queue.empty()) {
                dest_queue.push(file_name_1);
                cout << file_name_1 << " directly into dest_queue" << endl;
            }
            else {
                string file_name_2 = source_queue.front();
                source_queue.pop();

                string file_name_target = generate_short_filename(round); // 使用较短的标识符作为新文件名
                merge_sort_file(file_name_1, file_name_2, file_name_target);
                dest_queue.push(file_name_target);
            }
        }

        print_queue("dest_queue", dest_queue);
        cout << endl;

        source_queue = dest_queue;
        clear_queue(dest_queue); //必须clear
        round++;
    }

    // 如果最后一轮只有一个文件，直接将其作为最终结果
    if (!source_queue.empty()) {
        file_name_final = source_queue.front();
    }

    return file_name_final;
}
void create_index_file(const string& input_file, const string& output_file){
    ifstream inputFile(input_file, ios::binary); // 必须用二进制打开
    ofstream indexFile(output_file);

    if (!inputFile.is_open() || !indexFile.is_open()){
        cout << "Error: Unable to open inputFile or indexFile" << endl;
        return;
    }
    map<string, map<string, streampos>> index;
    string line;
    streampos offset = 0;
    while (getline(inputFile, line)){
        stringstream ss(line);
        string ts_code, trade_date;
        getline(ss, ts_code, ',');
        getline(ss, trade_date, ',');
        string month = trade_date.substr(0, 6);
        if (index[ts_code].find(month) == index[ts_code].end()){
            index[ts_code][month] = offset;
        }
        offset = inputFile.tellg();
    }

    for (const auto& stock : index){
        for (const auto& entry : stock.second){
            indexFile << stock.first << "," << entry.first << "," << entry.second << endl;
        }
    }

    cout << "Index file has been created successfully" << endl;
    inputFile.close();
    indexFile.close();
}

int main(int argc, char** argv){
    string file_input_stock = "input100.csv";
    int run_size_stock = 1000;

    string file_name_final = multi_round_2_way_merge_sort(file_input_stock,run_size_stock);

    ofstream outputFile("output.txt");
    if(!outputFile.is_open()){
        cout<<"Error: Unable to open output.txt"<<endl;
        return 1;
    }

    ifstream sortedFile(file_name_final);
    if(sortedFile.is_open()){
        string line;
        while(getline(sortedFile,line)){
            outputFile<<line<<endl;
        }
        sortedFile.close();
        cout<<"Sorting result has been written to output.txt"<<endl;
    }else{
        cout<<"Error: Unable to open sorted file"<<file_name_final<<endl;
        return 1;
    }

    string file_output_stock="output.txt";
    string file_output_index="index.txt";
    create_index_file(file_output_stock,file_output_index);
    outputFile.close();

    return 0;
}

