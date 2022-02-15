#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <unordered_map>
#include "json.hpp"
#define MAX_LEN 32
#define half_note 1
#define half_none_note 2
#define char_note 3
#define string_note 4
#define line_note 5
#define block_note 6
using json = nlohmann::json;
using namespace std;
unordered_map<string, int> varia;
unordered_map<string, int> reserve_word;
unordered_map<string, int> relational_operator;
unordered_map<string, int> logic_operator;
unordered_map<string, int> assignment_operator;
unordered_map<string, int> arithmetic_operator;
unordered_map<string, int> other_operator;
unordered_map<string, int> int_num;
unordered_map<string, int> float_num;
int lines, char_num;
void get_operator(string cur, unordered_map<string, int> &operator_table);
bool is_digit(char x)
{
    if (x <= '9' && x >= '0')
        return true;
    return false;
}
bool is_letter(char x)
{
    if ((x <= 'z' && x >= 'a') || (x <= 'Z' && x >= 'A'))
        return true;
    return false;
}
bool is_annotation_char(char x, int &is_note)
{
    //当且仅当当前符号需要重新判定是否为其他符号时返回false
    if (!is_note)
        switch (x)
        {
        case '\'':
            is_note = char_note;
            break;
        case '\"':
            is_note = string_note;
            break;
        case '/':
            is_note = half_note;
            break;
        default:
            return false;
            break;
        }
    else if (is_note == half_none_note)
    {
        if (x == '/')
            is_note = 0;
        else
            is_note = block_note;
    }
    else if (is_note == half_note)
    {
        switch (x)
        {
        case '/':
            is_note = line_note;
            break;
        case '*':
            is_note = block_note;
            break;
        default:
            is_note = 0;
            get_operator("/", arithmetic_operator);
            return false;
            break;
        }
    }
    return true;
}
int get_digit(string cur, int head)
{
    bool is_dot = false;
    string cur_digit;
    if(cur[head] == '-')
    {
        cur_digit += cur[head];
        head++;
    }
    for(;head < cur.length(); head++)
    {
        if(is_digit(cur[head]))
            cur_digit += cur[head];
        else if(!is_dot && cur[head] == '.')
        {
            is_dot = true;
            cur_digit += cur[head];
        }
        else
            break;
    }
    if (head < cur.length() && (is_letter(cur[head]) || cur[head] == '.'))
    {
        cout << "error : " << cur_digit + cur[head] << " in lines " << lines << endl;
        head--;
        cur_digit.pop_back();
    }
    auto iter = int_num.begin();
    if (!is_dot)
    {
        if ((iter = int_num.find(cur_digit)) != int_num.end())
            iter->second++;
        else
            int_num.insert({cur_digit, 1});
    }
    else
    {
        if ((iter = float_num.find(cur_digit)) != float_num.end())
            iter->second++;
        else
            float_num.insert({cur_digit, 1});
    }
    return head;
}
int get_string(string cur, int head)
{
    string result = "";
    for (; head < cur.length(); head++)
    {
        if (cur[head] == '_' || is_letter(cur[head]) || is_digit(cur[head]))
            result += cur[head];
        else
            break;
    }
    auto iter1 = reserve_word.begin();
    if ((iter1 = reserve_word.find(result)) != reserve_word.end())
        iter1->second++;
    else if ((iter1 = varia.find(result)) != varia.end())
        iter1->second++;
    else
        varia.insert({result, 1});
    return head;
}
int get_annotation(string cur, int head, int &is_note)
{
    char flag;
    switch (is_note)
    {
    case line_note:
        flag = '\n';
        break;
    case char_note:
        flag = '\'';
        break;
    case string_note:
        flag = '\"';
        break;
    case block_note:
        flag = '*';
        break;
    default:
        break;
    }
    while (head < cur.length() && cur[head] != flag)
    {
        if (cur[head] == '\n')

            lines++;
        head++;
    }
    if (head < cur.length())
    {
        if (cur[head] == '\n')
            lines++;
        head++;
        if (is_note != block_note)
            is_note = 0;
        else if (is_note == block_note)
            is_note = half_none_note;
    }
    return head;
}
void get_operator(string cur, unordered_map<string, int> &operator_table)
{
    auto iter = operator_table.begin();
    if ((iter = operator_table.find(cur)) != operator_table.end())
        iter->second++;
    else
        operator_table.insert({cur, 1});
}
int resolve_operator(string cur, int head)
{
    string tmp_operator;
    tmp_operator += cur[head];
    char state = cur[head];
    switch (state)
    {
    case '=':
        // ==
        if (cur[head + 1] == state)
        {
            tmp_operator += cur[head + 1];
            get_operator(tmp_operator, relational_operator);
        }
        // =
        else
        {
            get_operator(tmp_operator, assignment_operator);
        }
        break;
    case '>':
    case '<':
        if (cur[head + 1] == state)
        {
            tmp_operator += cur[head + 1];
            // >>= or <<=
            if (cur[head + 2] == '=')
            {
                tmp_operator += cur[head + 2];
                get_operator(tmp_operator, assignment_operator);
            }
            // >> or <<
            else
            {
                get_operator(tmp_operator, arithmetic_operator);
            }
        }
        // >= or <=
        else if (cur[head + 1] == '=')
        {
            tmp_operator += cur[head + 1];
            get_operator(tmp_operator, relational_operator);
        }
        // > or <
        else
        {
            get_operator(tmp_operator, relational_operator);
        }
        break;
    case '-':
        if (cur[head + 1] == '>')
        {
            tmp_operator += cur[head + 1];
            get_operator(tmp_operator, other_operator);
        }
    case '+':
        // ++ or --
        if (cur[head + 1] == state)
        {
            tmp_operator += cur[head + 1];
            get_operator(tmp_operator, arithmetic_operator);
        }
        // += or -=
        else if (cur[head + 1] == '=')
        {
            tmp_operator += cur[head + 1];
            get_operator(tmp_operator, assignment_operator);
        }
        // + or -
        else
        {
            get_operator(tmp_operator, arithmetic_operator);
        }
        break;
    case '&':
    case '|':
        // && or ||
        if (cur[head + 1] == state)
        {
            tmp_operator += cur[head + 1];
            get_operator(tmp_operator, logic_operator);
        }
        // &= or |=
        else if (cur[head + 1] == '=')
        {
            tmp_operator += cur[head + 1];
            get_operator(tmp_operator, assignment_operator);
        }
        // & or |
        else
        {
            get_operator(tmp_operator, arithmetic_operator);
        }
        break;
    case '*':
    case '/':
    case '^':
    case '%':
        // *= or /= or ^= or %=
        if (cur[head + 1] == '=')
        {
            tmp_operator += cur[head + 1];
            get_operator(tmp_operator, assignment_operator);
        }
        // * or / or ^ or %
        else
            get_operator(tmp_operator, arithmetic_operator);
        break;
    case '!':
        // !=
        if (cur[head + 1] == '=')
        {
            tmp_operator += cur[head + 1];
            get_operator(tmp_operator, relational_operator);
        }
        // !
        else
            get_operator(tmp_operator, logic_operator);
        break;
    case '?':
    case '{':
    case '}':
    case '[':
    case ']':
    case '(':
    case ')':
    case '.':
    case ';':
    case ':':
    case '#':
        get_operator(tmp_operator, other_operator);
        break;
    case '\n':
        lines++;
    case ' ':
    case '\t':
        break;
    default:
        break;
    }
    return head + tmp_operator.length();
}
int resolve(string cur, int head, int &is_note)
{
    if (is_note && is_note != half_note && is_note != half_none_note)
        return get_annotation(cur, head, is_note);
    else if (is_annotation_char(cur[head], is_note))
        return head + 1;
    else if (is_digit(cur[head]) || (cur[head] == '-' && is_digit(cur[head + 1])))
        return get_digit(cur, head);
    else if (cur[head] == '_' || is_letter(cur[head]))
        return get_string(cur, head);
    else
        return resolve_operator(cur, head);
}
void init()
{
    varia.empty();
    reserve_word.empty();
    relational_operator.empty();
    logic_operator.empty();
    assignment_operator.empty();
    arithmetic_operator.empty();
    other_operator.empty();
    int_num.empty();
    float_num.empty();
    json data, reserve_w;
    ifstream("mess.json") >> data;
    reserve_w = data["reserve_word"];
    for (auto i = reserve_w.begin(); i != reserve_w.end(); i++)
        reserve_word.insert({(*i), 0});
}
void output()
{
    ofstream outFile("result.txt", ios::out);
    auto iter_string = reserve_word.begin();
    outFile << left << setw(30) << "reserve word : ";
    outFile << left << "num: " << endl;
    for (; iter_string != reserve_word.end(); iter_string++)
    {
        if (iter_string->second)
        {
            outFile << left << setw(30) << iter_string->first;
            outFile << left << iter_string->second << endl;
        }
    }

    outFile << endl;
    iter_string = relational_operator.begin();
    outFile << left << setw(30) << "relational operator : ";
    outFile << left << "num: " << endl;
    for (; iter_string != relational_operator.end(); iter_string++)
    {
        if (iter_string->second)
        {
            outFile << left << setw(30) << iter_string->first;
            outFile << left << iter_string->second << endl;
        }
    }

    outFile << endl;
    iter_string = arithmetic_operator.begin();
    outFile << left << setw(30) << "arithmetic operator : ";
    outFile << left << "num: " << endl;
    for (; iter_string != arithmetic_operator.end(); iter_string++)
    {
        if (iter_string->second)
        {
            outFile << left << setw(30) << iter_string->first;
            outFile << left << iter_string->second << endl;
        }
    }

    outFile << endl;
    iter_string = logic_operator.begin();
    outFile << left << setw(30) << "logic operator : ";
    outFile << left << "num: " << endl;
    for (; iter_string != logic_operator.end(); iter_string++)
    {
        if (iter_string->second)
        {
            outFile << left << setw(30) << iter_string->first;
            outFile << left << iter_string->second << endl;
        }
    }

    outFile << endl;
    iter_string = assignment_operator.begin();
    outFile << left << setw(30) << "assignment operator : ";
    outFile << left << "num: " << endl;
    for (; iter_string != assignment_operator.end(); iter_string++)
    {
        if (iter_string->second)
        {
            outFile << left << setw(30) << iter_string->first;
            outFile << left << iter_string->second << endl;
        }
    }

    outFile << endl;
    iter_string = other_operator.begin();
    outFile << left << setw(30) << "other operator : ";
    outFile << left << "num: " << endl;
    for (; iter_string != other_operator.end(); iter_string++)
    {
        if (iter_string->second)
        {
            outFile << left << setw(30) << iter_string->first;
            outFile << left << iter_string->second << endl;
        }
    }

    outFile << endl;
    iter_string = varia.begin();
    outFile << left << setw(30) << "user variable : ";
    outFile << left << "num: " << endl;
    for (; iter_string != varia.end(); iter_string++)
    {
        if (iter_string->second)
        {
            outFile << left << setw(30) << iter_string->first;
            outFile << left << iter_string->second << endl;
        }
    }

    outFile << endl;
    iter_string = int_num.begin();
    outFile << left << setw(30) << "integer num : ";
    outFile << left << "num: " << endl;
    for (; iter_string != int_num.end(); iter_string++)
    {
        outFile << left << setw(30) << iter_string->first;
        outFile << left << iter_string->second << endl;
    }

    outFile << endl;
    iter_string = float_num.begin();
    outFile << left << setw(30) << "float num : ";
    outFile << left << "num: " << endl;
    for (; iter_string != float_num.end(); iter_string++)
    {
        outFile << left << setw(30) << iter_string->first;
        outFile << left << iter_string->second << endl;
    }

    outFile << endl;
    outFile << "lines: " << lines << endl;
    outFile << "character num: " << char_num << endl;
}
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "error command!" << endl;
        return 0;
    }
    init();
    string buf1, buf2;
    int head;
    bool is_head_in_buf1 = 1;
    ifstream inFile(argv[1], ios::in);
    int is_note = 0;
    inFile.clear();
    lines = 1;
    char_num = 0;
    char c;
    while (inFile.get(c))
    {
        char_num++;
        if (buf1.length() < 32 || buf2.length() < 32)
        {
            if (buf1.length() < 32)
                buf1 += c;
            else
                buf2 += c;
        }
        if (buf1.length() == 32 && buf2.length() == 32)
        {
            string cur;
            if (is_head_in_buf1)
                cur = buf1 + buf2;
            else
                cur = buf2 + buf1;
            while (head < 32)
            {
                head = resolve(cur, head, is_note);
            }
            while (head >= 32)
                head -= 32;
            if (is_head_in_buf1)
                buf1 = "";
            else
                buf2 = "";
            is_head_in_buf1 = !is_head_in_buf1;
        }
    }
    string cur;
    if (is_head_in_buf1)
        cur = buf1 + buf2;
    else
        cur = buf2 + buf1;
    while (head < cur.length())
        head = resolve(cur, head, is_note);
    output();
}