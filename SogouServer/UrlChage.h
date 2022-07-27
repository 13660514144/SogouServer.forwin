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
		std::string businesstype;//业务类型  由次类型定义下游IP PORT
	}urllist;
};


