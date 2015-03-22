//Dan Krupsky, 108

/*
�� �������� ����� MinGW
http://sourceforge.net/projects/boost/files/boost/1.57.0/ - ��� ������� (zip ����)
!!! ��� ���������� BOOST:
�������� � PATH ���� � MinGW\bin
����� � ���������� ����� tools\build\v2
���������
bootstrap mingw
tools\build\v2\b2 toolset=gcc --build-type=complete stage --with-filesystem
���������� ����� � ���������� �����  \ stage\lib
������������� ��������������� .a ����� (��� ����� filesystem � system) ������������� � libboost_filesystem.a � libboost_system.a
!!! ��� ���������� ��������:
����������� �������� � home ���������� msys'a � mingw, ����� � ��� ����������
./configure --prefix=`cd C:/MinGW; pwd -W`
make
make install

����� � ����� � �����������, ������ proto ���� ����� ������������� �������� protoc:
protoc example.proto
�� ������ ���������� ��� �����: example.pb.h � example.pb.cc, � main.cpp ������ ������ �� .h ����

!!! ������� ��� ���������� (�� MSYS):
g++ main.cpp pbfile.pb.cc -std=c++11 -IC:/MinGW/msys/1.0/home/boost_1_57_0/ -LC:/MinGW/msys/1.0/home/boost_1_57_0/stage/lib -lboost_filesystem -lboost_system -lprotobuf

��� ������ �� �� msys'a ���� ����� ��������� dll � ���������� mingw � ������� �� � ����� � exe ������:
libgcc_s_dw2-1.dll
libprotobuf-9.dll
libstdc++-6.dll
*/

#include <iostream>
#include <fstream>  //������ � ������
#include <string>
#include <boost/filesystem.hpp>  //BOOST::FILESYSTEM � ������� �������� ��������� ����������
#include "pbfile.pb.h" //����, ������� ������ protoc
#include "md6/md6.h"

namespace fs = boost::filesystem;
std::string test;

//������ ����� ���������� � ���������, � ��������� ���������� � ������
struct Fileinfo {
	std::string path;
	std::string hash;
	int size;
	std::string flag;
};

//���������� ����� ��������. Filelist - ������� ���������, � ������� ������ �������� Filep
void savepbuf(std::string filename, std::vector<Fileinfo> vec_finfo) {
	nsofdir::ArrFilep flist;
	nsofdir::Filep * file_entry;
	std::ofstream output(filename, std::ofstream::binary);
	for (Fileinfo it : vec_finfo) {
		//������, ������ �� ��������� ������� ���������
		file_entry = flist.add_filep();
		file_entry->set_filepath(it.path);
		file_entry->set_size(it.size);
		file_entry->set_mdsixhash(it.hash); // �������� �����

	}
	//����� �����
	flist.PrintDebugString();
	//���������� � output ����
	flist.SerializeToOstream(&output);
	output.close();
}

//�������� ����������� ����� (���������� � ����, �� ���� �� ���������� � ������ - �������� �����
void loadpbuf(std::string filename, std::vector<Fileinfo> vec_finfo) {
	nsofdir::ArrFilep flist;  //Filelist, � ������� ������� ����
	nsofdir::Filep * file_entry;
	std::ifstream input("filelist.pb", std::ofstream::binary); // ��������� ��� ���������� ����
	flist.ParseFromIstream(&input);  //������ �� �����
	//flist.PrintDebugString(); // ����� �����
	input.close();
}

//������� ������ ������ � ����� � ����������
void get_dir_list(fs::directory_iterator iterator, std::vector<Fileinfo> & vec_finfo, Fileinfo & finfo, std::ifstream & ifs) {
	for (; iterator != fs::directory_iterator(); ++iterator)
	{
		if (fs::is_directory(iterator->status())) {
			//���� ���������� �� �����, �� ���������� ��������� ��� �� ������� ��� ���� �����
			fs::directory_iterator sub_dir(iterator->path());
			get_dir_list(sub_dir, vec_finfo, finfo, ifs);

		}
		else
			//� ���� ���, �� ���������� � ��������� ���, ������, ���, � ������ (����������� ���� �����, ����� ����� ������ ��������� � ������)
		{

			finfo.path = iterator->path().string();
			//���������� ����� � \ ����� ������ � ����, ��� �������� filesystem :-(
			std::replace(finfo.path.begin(), finfo.path.end(), '\\', '/');
			finfo.size = fs::file_size(iterator->path());
			ifs.open(finfo.path, std::ios_base::binary);
			std::string strifs((std::istreambuf_iterator<char>(ifs)),
				(std::istreambuf_iterator<char>()));
			finfo.hash = md6(strifs);
			ifs.close();
			finfo.flag = 'R';
			vec_finfo.push_back(finfo);
		}

	}
}

int main() {
	std::ofstream myfile;
	std::string path, dirpath;
	//���������� ���������, � ������� ����� ���������� ������ � ���������� �� � ������
	Fileinfo finfo;
	//�����, ����� ������� �� ������� ���
	std::ifstream ifs;
	std::cout << "Folder path:" << std::endl;
	std::getline(std::cin, path);
	// ������, ���� ����� ����������� � ��������� ����� ����, ������ �� ������ ������������ ���������� � ����� ������� ����.
	// ���� ����� ������� �� ������ - �������� - ��������.
	//������, � ������� �� ����� ���������� ������� ����� ���������
	std::vector<Fileinfo> vec_finfo;
	//������, � ������� �� ����� ��������� �� �����, ����� �������� ��� ������ ������
	std::vector<Fileinfo> vec_finfo_old;
	//��������, ������� ����� ������ �� ������ � ������
	fs::directory_iterator home_dir(path);
	//������ �������, ������� ������� � ������ ��� �����
	get_dir_list(home_dir, vec_finfo, finfo, ifs);

	//������� ������ ������, �������� � �.�. �� �������. ������ ��� ������.
	/*
	for (Fileinfo element : vec_finfo) {
	std::cout << element.path << std::endl <<
	element.size << std::endl <<
	element.hash << std::endl <<
	element.flag << std::endl;
	}
	*/
	//��������� ���������� � ��� ����� � ������ example.xml, ��������� �� � ����� ��� ��������� main.cpp
	savepbuf("filelist.pb", vec_finfo);
	//loadpbuf("example.xml", vec_finfo_old);  //�������� �����
	std::cin.clear();
	fflush(stdin);
	std::cin.get();
	return 0;
}