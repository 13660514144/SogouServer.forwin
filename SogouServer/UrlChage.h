#pragma once
#include<iostream>
#include<list>
#include<string>
#include<iterator>
#include<algorithm>
class UrlChage
{
public:
    struct URL {
		std::string domain;
		std::string port;
		std::string controller;
		std::string paras;
		std::string businesstype;//ҵ������  �ɴ����Ͷ�������IP PORT
	}urllist;
};


