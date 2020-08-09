#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>
#include <vector>

#define MEGABYTE 1048576

using namespace std;
namespace fs = filesystem;

struct file_part
{
	file_part(const string& name_)
	{
		name = name_;
	}

	file_part(const string& name_, char* data_, uint32_t size)
	{
		name = name_;

		copy(data_, data_ + size, back_inserter(data));
	}
	string name;
	vector<char> data;
};

void split_file(const string& file, int block_size = MEGABYTE);
void join_file(const string& file_name);

int main(int argc, char* argv[])
{
	string file_name;
	bool split;

	if (argc < 3 || argv[1][0] == 'h')
	{
		cout << "Usage:\n"
			<< "\tFileSplitter.exe h									- show help\n"
			<< "\tFileSplitter.exe <file name> split					- split file into blocks of 1 Mb each\n"			
			<< "\tFileSplitter.exe <file name> split <block size in Kb> - split file into blocks of custom size\n"
			<< "\tFileSplitter.exe <file name> join						- join all blocks into one file, you need to specify only first (_0) file name\n";
		return 0;
	}
	
	file_name = argv[1];
	split = strcmp(argv[2], "split") == 0;
	
	if (split)
	{
		int block_size = MEGABYTE;
		if (argc == 4)
		{
			block_size = stoi(argv[3]) * 1024;
		}
		split_file(file_name, block_size);
	}
	else
	{
		if (file_name[file_name.length() - 1] != '0')
		{
			cout << "Error while checking file name (<file name>_0)\n";
			return 0;
		}	
		join_file(file_name);
	}

	cout << "Done, press any key to exit...";
	cin.get();

	return 0;
}

void split_file(const string& file_name, int block_size)
{
	vector<file_part> splitted;
	ifstream file(file_name, ios::binary | ios::ate);
	auto file_size = file.tellg();
	cout << "File size: " << file_size / MEGABYTE << " mb. (" << file_size << " bytes)" << '\n';
	file.close();
	file = ifstream(file_name, ios::binary);

	auto parts_amount = (file_size % block_size == 0 ? file_size / block_size : file_size / block_size + 1);
	cout << "Amount of parts: " << parts_amount << '\n';

	int count = 0;
	while (parts_amount > count)
	{
		char* data = new char[block_size];
		int diff = file_size - file.tellg();
		int size = block_size;
		if (diff < block_size)
		{
			size = diff;
		}
		file.read(data, size);
		splitted.emplace_back(file_name + "_" + to_string(count), data, size);
		cout << "Read: " << file.tellg() << " bytes\n";
		++count;
		delete[] data;
	}
	file.close();
	cout << "Writing...\n";
	count = 0;

	for (const auto& i : splitted)
	{
		ofstream output(i.name, ios::binary);
		cout << "Writing file " << count << ": " << i.name << '\n';
		output.write(i.data.data(), i.data.size());
		++count;
		output.close();
	}
}

void join_file(const string& file_name)
{
	vector<file_part> splitted;
	int counter_index = file_name.find_last_of("_");
	string file_name_ = file_name.substr(0, counter_index);
	cout << "File name: " << file_name_ << '\n';

	fs::directory_iterator beg_dir{ fs::current_path() };
	fs::directory_iterator end_dir{};
	for (auto i = beg_dir; i != end_dir; ++i) // заполняем вектор именами частей
	{
		auto f_name = i->path().filename().string();
		if (f_name.find(file_name_) != string::npos && isdigit(f_name.back()))
		{
			splitted.emplace_back(f_name);
		}
	}

	sort(splitted.begin(), splitted.end(), [](const file_part& part1, const file_part& part2)
		{
			int index1, index2;
			index1 = part1.name.find_last_of("_") + 1;
			index2 = part2.name.find_last_of("_") + 1;

			int number1, number2;
			number1 = stoi(part1.name.substr(index1));
			number2 = stoi(part2.name.substr(index2));
			return number1 < number2;
		});

	cout << "Reading...\n";
	int mem_read = 0;
	for (auto& i : splitted)
	{
		ifstream fl(i.name, ios::binary | ios::ate);
		auto fl_size = fl.tellg();
		fl.close();
		fl = ifstream(i.name, ios::binary);
		char* data = new char[fl_size];
		fl.read(data, fl_size);

		copy(data, data + fl_size, back_inserter(i.data));
		mem_read += fl_size;

		cout << "File size: " << fl_size / MEGABYTE << " Mb. (" << fl_size << " bytes)" << '\n';
		cout << "Total read: " << mem_read << " bytes\n";
		fl.close();
		delete[] data;
	}

	cout << "Writing...\n";
	ofstream output_file(file_name_, ios::binary);

	int count = 0;
	int mem_write = 0;
	for (const auto& i : splitted)
	{
		mem_write += i.data.size();
		cout << "Writing block " << count << ": " << i.data.size() << " bytes\n";
		cout << "Total written " << mem_write << " bytes\n";
		output_file.write(i.data.data(), i.data.size());
		++count;
	}

	output_file.close();
}