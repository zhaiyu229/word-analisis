#include<stdio.h>
#include<iostream>
#include<vector>
#include<string>
#include<fstream>
#define bufferSize 512		//缓冲区大小
using namespace std;
int state;			//当前状态指示
char C;				//当前读入字符
string nowstr;		//当前读入的字符串
char buffer[bufferSize];	//输入缓冲区
int forwar = -1;  		//向前指针
int rows = 1;			//文件行数
int sum_char = 0;		//文件总字符数

struct token {				
	string mark;		//记号
	string name;		//属性
	int count;			//出现次数
};						//符号表格式

vector<string> keyword  = {"auto","break","case","char","const","continue","default","do","double","else","enum","extern",
						   "float","for","goto","if","inline","int","long","register","restrict","return","short","signed",
							"sizeof","static","struct","switch","typedef","union","unsigned","void","volatile","while",
							"_Alignas","_Alignof","_Atomic","_Bool","_Complex","_Generic","_Imaginary","_Noreturn","_Static_assert"
							"_Thread_local"};
void get_char() {
	forwar=(forwar+1)%bufferSize;
	C = buffer[forwar];
}
//从buf中读一个字符到C中，向前指针移动。


void cat() {
	nowstr.push_back(C);
}
//将字符C连接到nowstr字符串后面

bool is_letter(char ch) {
	if (isalpha(ch) || ch=='_')
		return true;
	else
		return false;
}
//判断ch是否为字母或下划线

bool iskeyword() {
	for (int i = 0; i < keyword.size(); i++)
		if (nowstr == keyword[i]) {
			return true;
		}
	return false;
}
//判断nowstr中是否为关键字

bool iseven() {
	int num = 0;
	int i = nowstr.size() - 2;
	while (nowstr[i] == '\\') {
		num++;
		i--;
	}
	if (num % 2 == 0)
		return true;
	return false;
}
//判断后引导前面‘\’的奇偶



void addToken(string a, string b, vector<token>& tempresult) {
	int i;
	for (i = 0; i < tempresult.size(); i++)
		if (tempresult[i].mark == a && tempresult[i].name == b){	
			tempresult[i].count++;
			break;
		}
	if (i == tempresult.size()) {
		token t;
		t.mark = a;
		t.name = b;
		t.count = 1;
		tempresult.push_back(t);
	}
}
//将识别出的单词加入符号表


vector<token> analisis(ifstream& f) {
	vector<token> result;
	bool isEnd = false;
	f.read(buffer, bufferSize - 1);
	if (f.gcount() < bufferSize - 1) {
		buffer[f.gcount()] = EOF;
	}
	buffer[bufferSize - 1] = EOF;
	state = 0;
	while (!isEnd) {
		get_char();
		if (C == '\n')
			rows++;
		if (C != EOF) {
			sum_char++;
		}
		if (C == EOF && forwar != bufferSize - 1)
			isEnd = true;
		else if (C == EOF && forwar == bufferSize - 1) {
			f.read(buffer, bufferSize - 1);
			if (f.gcount() < bufferSize - 1) {
				buffer[f.gcount()] = EOF;
			}
			continue;
		}
		switch (state)
			{case 0:				//初始状态
				if (is_letter(C)) {
					state = 1;		//设置标识符状态
					cat();
				}
				else if (isdigit(C)&&C!='0') {
					state = 2;		//设置常数状态
					cat();
				}
				else if (C == '0') {
					state = 28;
					cat();
				}
				else {
					switch (C) {
					case '<': state = 8; break;
					case '>': state = 9; break;
					case'?':  addToken("delimeter           ", "?", result); break;
					case ':': addToken("delimeter           ", ":", result); break;
					case '/': state = 11; break;
					case '=': state = 12; break;
					case '+': state = 13; break;
					case '-': state = 14; break;
					case '*': state = 15; break;
					case '%': state = 16; break;
					case '(': addToken("delimeter           ", "(", result); break;
					case ')': addToken("delimeter           ", ")", result); break;
					case ',': addToken("delimeter           ", ",", result); break;
					case ';': addToken("delimeter           ", ";", result); break;
					case '{': addToken("delimeter           ", "{", result); break;
					case '}': addToken("delimeter           ", "}", result); break;
					case '[': addToken("delimeter           ", "[", result); break;
					case ']': addToken("delimeter           ", "]", result); break;
					case '^': state = 17; break;
					case '|': state = 18; break;
					case '~': state = 19; break;
					case '!': state = 20; break;
					case '&': state = 21; break;
					case '"': state = 22; cat(); break;
					case '\'':state = 23; cat(); break;
					case '.': state = 24; break;
					case ' ':
					case '\n':
					case'	':
					case EOF : break; //跳过空白符
					default: cout << "error:第" << rows << "行出现非法字符" << C << endl;
						break;
					}
				}
				break;
			case 1:				//标识符状态
				if (is_letter(C) || isdigit(C)) {
					cat();
					state = 1;
				}
				else {
					forwar--;
					sum_char--;
					state = 0;
					if (C == '\n')
						rows--;
					if (iskeyword()) {
						addToken("keyword             ", nowstr, result);
					}
					else
						addToken("id                  ", nowstr, result);
					nowstr.clear();
				}
				break;
			case 2:				//常数状态
				if (isdigit(C)) {
					state = 2;
					cat();
				}
				else {
					switch (C) {
					case '.':cat(); state = 3; break;
					case 'E':
					case 'e':cat(); state = 5; break;
					default:
						forwar--;
						sum_char--;
						if (C == '\n')
							rows--;
						state = 0;
						addToken("intenger            ", nowstr, result);
						nowstr.clear();
						break;
					}
				}
				break;
			case 3:				//小数点状态
				if (isdigit(C)) {
					cat();
					state = 4;
				}
				else {
					forwar--;
					sum_char--;
					if (C == '\n')
						rows--;
					nowstr.push_back('0');
					addToken("float               ", nowstr, result);
					state = 0;
					nowstr.clear();
				}
				break;
			case 4:			//小数状态
				if (isdigit(C)) {
					cat();
					state = 4;
				}
				else if (C == 'e'||C=='E') {
					state = 5;
					cat();
				}
				else {
					forwar--;
					sum_char--;
					if (C == '\n')
						rows--;
					state = 0;
					addToken("float               ", nowstr, result);
					nowstr.clear();
				}
				break;
			case 5:			//指数状态
				if (isdigit(C)) {
					cat();
					state = 7;
				}
				else if (C == '+' || C == '-') {
					cat();
					state = 6;
				}
				else {
					forwar--;
					sum_char--;
					if (C == '\n')
						rows--;
					cout << "error:第" << rows << "行缺少指数" << endl;//error();
					state = 0;
					nowstr.clear();
				}
				break;
			case 6:
				if (isdigit(C)) {
					cat();
					state = 7;
				}
		
				else {
					forwar--;
					sum_char--;
					if (C == '\n')
						rows--;
					cout << "error:第" << rows << "行缺少指数" << endl;
					state = 0;
					nowstr.clear();
				}
				break;
			case 7:
				if (isdigit(C)) {
					cat();
					state = 7;
				}
				else {
					forwar--;
					sum_char--;
					if (C == '\n')
						rows--;
					state = 0;
					addToken("exponent            ", nowstr, result);
					nowstr.clear();
				}
				break;
			case 8:
				if (C == '=') {
					addToken("relational operator ", "<=", result);
					state = 0;
				}
				else if (C == '<') {
					addToken("bit operator        ", "<<", result);
					state = 0;
				}
				else {
					addToken("relational operator ", "<", result);
					if (C == '\n')
						rows--;
					forwar--;
					sum_char--;
					state = 0;
				}
				break;
			case 9:
				if (C == '=') {
					addToken("relational operator ", ">=", result);
					state = 0;
				}
				else if (C == '>') {
					addToken("bit operator        ", ">>", result);
					state = 0;
				}
				else {
					addToken("relational operator ", ">", result);
					if (C == '\n')
						rows--;
					forwar--;
					sum_char--;
					state = 0;
				}
				break;
	
			case 11:				//  /状态
				switch (C) {
				case '/':
					state = 27;
					break;
				case '*':
					state = 25;
					break;
				case '=':
					addToken("assign operator     ", "/=", result);
					state = 0;
					break;
				default:
					addToken("arithmetic operator ", "/", result);
					state = 0;
					if (C == '\n')
						rows--;
					forwar--;
					sum_char--;
					break;
				}
				break;
			case 25:			// /*状态
				if (C == '*')
					state = 26;
				else state = 25;
				break;
			case 26:			// /*..*状态
				if (C == '/')
					state = 0;
				else
					state = 25;
				break;
			case 27:			//   //状态
				if (C == '\n')
					state = 0;
				else
					state = 27;
				break;
			case 12:			// =状态
				if (C == '=') {
					addToken("relational operator ", "==", result);
					state = 0;
				}
				else {
					addToken("assign operator     ", "=", result);
					state = 0;
					if (C == '\n')
						rows--;
					forwar--;
					sum_char--;
				}
				break;
			case 13:			//+状态
				if (C == '=') {
					addToken("assign operator     ", "+=", result);
					state = 0;
				}
				else if (C == '+') {
					addToken("arithmetic operator ", "++", result);
					state = 0;
				}
				else {
					addToken("arithmetic operator ", "+", result);
					state = 0;
					if (C == '\n')
						rows--;
					forwar--;
					sum_char--;
				}
				break;
			case 14:		//-状态
				if (C == '=') {
					addToken("assign operator     ", "-=", result);
					state = 0;
				}
				else if (C == '-') {
					addToken("arithmetic operator ", "--", result);
					state = 0;
				}
				else if (C == '>') {
					addToken("special operator    ", "->", result);
				}
				else {
					addToken("arithmetic operator ", "-", result);
					state = 0;
					if (C == '\n')
						rows--;
					forwar--;
					sum_char--;
				}
				break;
			case 15:				//*状态
				if (C == '=') {
					addToken("assign operator     ", "*=", result);
					state = 0;
				}
				else {
					addToken("special operator    ", "*", result);
					state = 0;
					if (C == '\n')
						rows--;
					forwar--;
					sum_char--;
				}
				break;
			case 16:				//%状态
				if (C == '=') {
					addToken("assign operator     ", "%=", result);
					state = 0;
				}
				else {
					addToken("arithmetic operator ", "%", result);
					state = 0;
					if (C == '\n')
						rows--;
					forwar--;
					sum_char--;
				}
				break;
			case 17:			//^
				if (C == '=') {
					addToken("assign operator     ", "^=", result);
					state = 0;
				}
				else {
					addToken("bit operator        ", "^", result);
					state = 0;
					if (C == '\n')
						rows--;
					forwar--;
					sum_char--;
				}
				break;
			case 18:			//|
				if (C == '=') {
					addToken("assign operator     ", "|=", result);
					state = 0;
				}
				else if (C == '|') {
					addToken("logical operator    ", "||", result);
					state = 0;
				}
				else {
					addToken("bit operator        ", "|", result);
					state = 0;
					if (C == '\n')
						rows--;
					forwar--;
					sum_char--;
				}
				break;
			case 19:			//~
				if (C == '=') {
					addToken("assign operator     ", "~=", result);
					state = 0;
				}
				else {
					addToken("bit operator        ", "~", result);
					state = 0;
					if (C == '\n')
						rows--;
					forwar--;
					sum_char--;
				}
				break;
			case 20:		//!
				if (C == '=') {
					addToken("relational operator ", "!=", result);
					state = 0;
				}
				else {
					addToken("logical operator    ", "!", result);
					state = 0;
					if (C == '\n')
						rows--;
					forwar--;
					sum_char--;
				}
				break;
			case 21:			//&
				if (C == '&') {
					addToken("logical operator    ", "&&", result);
					state = 0;
				}
				else {
					addToken("special operator    ", "&", result);
					state = 0;
					if (C == '\n')
						rows--;
					forwar--;
					sum_char--;
				}
				break;
			case 22:			//"
				if (C == '"') {
					cat();
					if (iseven()) {
						addToken("string              ", nowstr, result);
						nowstr.clear();
						state = 0;
					}
					else {
						state = 22;
					}
				}
				else {
					cat();
					state = 22;
				}
				break;
			case 23:			//'
				if (C == '\'') {
					cat();
					if (iseven()) {
						addToken("char                ", nowstr, result);
						nowstr.clear();
						state = 0;
					}
					else {
						state = 23;
					}
				}
				else {
					cat();
					state = 23;
				}
				break;
			case 24:			
				if (isdigit(C)) {
					nowstr.push_back('0');
					nowstr.push_back('.');
					cat();
					state = 4;
				}
				else {
					addToken("special operator    ", ".", result);
					if (C == '\n')
						rows--;
					forwar--;
					sum_char--;
					state = 0;
				}
				break;
			case 28:		//0状态
				if (C == 'x') {
					cat();
					state = 29;
				}
				else if (isdigit(C)) {
					cat();
					state = 2;
				}
				else {
					addToken("intenger            ", "0", result);
					if (C == '\n')
						rows--;
					forwar--;
					sum_char--;
					state = 0;
					nowstr.clear();
				}
				break;
			case 29:
				if (isdigit(C)) {
					cat();
					state = 2;
				}
				else {
					cout << "error:第" << rows << "行缺少16进制数字" << endl;//error();
					state = 0;
					forwar--;
					sum_char--;
					if (C == '\n')
						rows--;
					nowstr.clear();
				}
				break;
			default:
				break;
			}
	}
	return result;
}	
//主体部分，对文件f中的程序进行词法分析


void output(vector<token> result) {	
	int count_keyword = 0;
	int count_id = 0;
	int count_int = 0;
	int count_float = 0;
	int count_exponent = 0;
	int count_relationalOperator = 0;
	int count_logicOperator = 0;
	int count_bitOperator = 0;
	int count_assignOperator = 0;
	int count_specialOperator = 0;
	int count_arithmeticOperator = 0;
	int count_string = 0;
	int count_char = 0;
	int count_delimeter = 0;
	cout << "记号                " << "属性" <<"                          出现次数"<< endl;
	for (int i = 0; i < result.size(); i++) {
		int j = 30 - result[i].name.size();
		for (int k = 0; k < j; k++) {
			result[i].name.push_back(' ');
		}		
	}
	for (int i = 0; i < result.size(); i++)
		if (result[i].mark == "keyword             ") {
			cout << result[i].mark << result[i].name << result[i].count << endl;
			count_keyword++;
		}
	for (int i = 0; i < result.size(); i++)
		if (result[i].mark == "id                  ") {
			cout << result[i].mark << result[i].name << result[i].count << endl;
			count_id++;
		}
	for (int i = 0; i < result.size(); i++)
		if (result[i].mark == "intenger            ") {
			cout << result[i].mark << result[i].name << result[i].count << endl;
			count_int++;
		}
	for (int i = 0; i < result.size(); i++)
		if (result[i].mark == "float               ") {
			cout << result[i].mark << result[i].name << result[i].count << endl;
			count_float++;
		}
	for (int i = 0; i < result.size(); i++)
		if (result[i].mark == "exponent            ") {
			cout << result[i].mark << result[i].name << result[i].count << endl;
			count_exponent++;
		}
	for (int i = 0; i < result.size(); i++)
		if (result[i].mark == "relational operator") {
			cout << result[i].mark << result[i].name << result[i].count << endl;
			count_relationalOperator++;
		}
	for (int i = 0; i < result.size(); i++)
		if (result[i].mark == "logical operator    ") {
			cout << result[i].mark << result[i].name << result[i].count << endl;
			count_logicOperator++;
		}
	for (int i = 0; i < result.size(); i++)
		if (result[i].mark == "bit operator        ") {
			cout << result[i].mark << result[i].name << result[i].count << endl;
			count_bitOperator++;
		}
	for (int i = 0; i < result.size(); i++)
		if (result[i].mark == "assign operator     ") {
			cout << result[i].mark << result[i].name << result[i].count << endl;
			count_assignOperator++;
		}
	for (int i = 0; i < result.size(); i++)
		if (result[i].mark == "special operator    ") {
			cout << result[i].mark << result[i].name << result[i].count << endl;
			count_specialOperator++;
		}
	for (int i = 0; i < result.size(); i++)
		if (result[i].mark == "arithmetic operator ") {
			cout << result[i].mark << result[i].name << result[i].count << endl;
			count_arithmeticOperator++;
		}
	for (int i = 0; i < result.size(); i++)
		if (result[i].mark == "string              ") {
			cout << result[i].mark << result[i].name << result[i].count << endl;
			count_string++;
		}
	for (int i = 0; i < result.size(); i++)
		if (result[i].mark == "char                ") {
			cout << result[i].mark << result[i].name << result[i].count << endl;
			count_char++;
		}
	for (int i = 0; i < result.size(); i++)
		if (result[i].mark == "delimeter           ") {
			cout << result[i].mark << result[i].name << result[i].count << endl;
			count_delimeter++;
		}
	if (count_keyword > 0)
		cout << "count_keywords:  " << count_keyword << endl;
	if (count_id > 0)
		cout << "count_id :" << count_id << endl;
	if (count_int > 0)
		cout << "count_int:" << count_int << endl;
	if (count_float > 0)
		cout << "count_float:" << count_float << endl;
	if (count_exponent > 0)
		cout << "count_exponent:" << count_exponent << endl;
	if (count_relationalOperator > 0)
		cout << "count_relationalOperator:" << count_relationalOperator << endl;
	if (count_logicOperator > 0)
		cout << "count_logicOperator:" << count_logicOperator << endl;
	if (count_bitOperator > 0)
		cout << "count_bitOperator:" << count_bitOperator << endl;
	if (count_assignOperator > 0)
		cout << "count_assignOperator:" << count_assignOperator << endl;
	if (count_specialOperator > 0)
		cout << "count_specialOperator:" << count_specialOperator << endl;
	if (count_arithmeticOperator > 0)
		cout << "count_arithmeticOperator:" << count_arithmeticOperator << endl;
	if (count_string > 0)
		cout << "count_string:" << count_string << endl;
	if (count_char > 0)
		cout << "count_char:" << count_char << endl;
	if (count_delimeter > 0)
		cout << "count_delimeter:" << count_delimeter << endl;

	cout << "rows in total:" << rows << endl;
	cout << "chars in total:" << sum_char << endl;
}			
//输出结果

int main(void) {
	ifstream fs;
	fs.open("test.txt", ios::in);
	if (fs.is_open() == false)
		exit(0);
	vector<token> result = analisis(fs);
	fs.close();
	output(result);
	return 0;
}