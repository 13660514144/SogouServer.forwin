// SogouServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <utility>
#include <string>
#include <algorithm>
#include "workflow/Workflow.h"
#include "workflow/HttpMessage.h"
#include "workflow/HttpUtil.h"
#include "workflow/WFHttpServer.h"

#include "UrlChage.h"
#ifndef _WIN32
#include <unistd.h>
#endif
using namespace std;

struct tutorial_series_context
{
	std::string url;
	WFHttpTask* proxy_task;
	bool is_keep_alive;
};
UrlChage uclass;
list<UrlChage::URL> worklisturl;
//字符串分割到数组
void Split(const string& src, const string& separator, vector<string>& dest)
{
	string str = src;
	string substring;
	string::size_type start = 0, index;
	dest.clear();
	index = str.find_first_of(separator, start);
	do
	{
		if (index != string::npos)
		{
			substring = str.substr(start, index - start);
			dest.push_back(substring);
			start = index + separator.size();
			index = str.find(separator, start);
			if (start == string::npos) break;
		}
	} while (index != string::npos);

	//the last part
	substring = str.substr(start);
	dest.push_back(substring);
}

void reply_callback(WFHttpTask* proxy_task)
{
	SeriesWork* series = series_of(proxy_task);
	tutorial_series_context* context =
		(tutorial_series_context*)series->get_context();
	auto* proxy_resp = proxy_task->get_resp();
	size_t size = proxy_resp->get_output_body_size();

	if (proxy_task->get_state() == WFT_STATE_SUCCESS)
	{
		fprintf(stderr, "%s: Success. Http Status: %s, BodyLength: %zu\n",
			context->url.c_str(), proxy_resp->get_status_code(), size);
	}
	else
	{
		/* WFT_STATE_SYS_ERROR*/
		fprintf(stderr, "%s: Reply failed: %s, BodyLength: %zu\n",
			context->url.c_str(), strerror(proxy_task->get_error()), size);
	}
}

void http_callback(WFHttpTask* task)
{
	int state = task->get_state();
	int error = task->get_error();
	auto* resp = task->get_resp();
	SeriesWork* series = series_of(task);
	tutorial_series_context* context =
		(tutorial_series_context*)series->get_context();
	auto* proxy_resp = context->proxy_task->get_resp();
	
	/* Some servers may close the socket as the end of http response. */
	if (state == WFT_STATE_SYS_ERROR && error == ECONNRESET)
		state = WFT_STATE_SUCCESS;

	if (state == WFT_STATE_SUCCESS)
	{
		const void* body;
		size_t len;

		/* set a callback for getting reply status. */
		context->proxy_task->set_callback(reply_callback);

		/* Copy the remote webserver's response, to proxy response. */
		if (resp->get_parsed_body(&body, &len))
			resp->append_output_body_nocopy(body, len);
		*proxy_resp = std::move(*resp);
		//解决跨域问题
		proxy_resp->set_header_pair("Access-Control-Allow-Origin", "*");
		proxy_resp->set_header_pair("Access-Control-Allow-Methods", "*");
		if (!context->is_keep_alive)
			proxy_resp->set_header_pair("Connection", "close");
			
	}
	else
	{
		const char* err_string;
		int error = task->get_error();

		if (state == WFT_STATE_SYS_ERROR)
			err_string = strerror(error);
		else if (state == WFT_STATE_DNS_ERROR)
			err_string = "pause ,bucaouse thing dont ";// gai_strerror(error);
		else if (state == WFT_STATE_SSL_ERROR)
			err_string = "SSL error";
		else /* if (state == WFT_STATE_TASK_ERROR) */
			err_string = "URL error (Cannot be a HTTPS proxy)";

		fprintf(stderr, "%s: Fetch failed. state = %d, error = %d: %s\n",
			context->url.c_str(), state, task->get_error(),
			err_string);

		/* As a tutorial, make it simple. And ignore reply status. */
		proxy_resp->set_status_code("404");
		proxy_resp->append_output_body_nocopy(
			"<html>404 Not Found.</html>", 27);
	}
}

void process(WFHttpTask* proxy_task)
{
	//WFHttpServer server([](WFHttpTask* task) {
	//	task->get_resp()->append_output_body("<html>Hello World!</html>");
	//	});

	auto* req = proxy_task->get_req();
	SeriesWork* series = series_of(proxy_task);
	WFHttpTask* http_task; /* for requesting remote webserver. */
	//std::string http_body = protocol::HttpUtil::decode_chunked_body(&proxy_task->get_req());
	tutorial_series_context* context = new tutorial_series_context;
	context->url = req->get_request_uri();
	context->proxy_task = proxy_task;

	series->set_context(context);
	series->set_callback([](const SeriesWork* series) {
		delete (tutorial_series_context*)series->get_context();
		});

	std::string requesturi = req->get_request_uri();
	vector<string> Data;
	Split(requesturi, "/", Data);
	// add begain
	protocol::HttpHeaderCursor header_cursor(req);
	std::string host;
	/*if (header_cursor.find("Host", host))
	{
		requesturi = "http://" + host + requesturi;
	}*/
	string api = requesturi;
	string upserver = "/Api/PassRecoadGuest/PostFile,/Api/PassRecoadGuest/GetLastVer,/up.html,/ver.html";
	transform(upserver.begin(), upserver.end(), upserver.begin(), ::toupper);
	string rabbmqserver = "/api/ListPage/GetPage,/list.html,/CurrencySearch.html";
	transform(rabbmqserver.begin(), rabbmqserver.end(), rabbmqserver.begin(), ::toupper);
	int n1;
	transform(requesturi.begin(), requesturi.end(), requesturi.begin(), ::toupper);
	if ((n1= upserver.find(requesturi))!=string::npos)
	{
		api = "http://127.0.0.1:24160" + api;
	}
	else if((n1 = rabbmqserver.find(requesturi)) != string::npos)
	{
		api = "http://127.0.0.1:24200" + api;
	}
	else
	{
		api = "http://127.0.0.1:24200" + api;
	}
	printf("uri: %s\r\n", api.c_str());
	// add end

	context->is_keep_alive = req->is_keep_alive();
	http_task = WFTaskFactory::create_http_task(api.c_str(), 0, 0,http_callback);

	const void* body;
	size_t len;

	/* Copy user's request to the new task's reuqest using std::move() */
	std::string mothed=req->get_method();

	req->set_request_uri(http_task->get_req()->get_request_uri());
	req->get_parsed_body(&body, &len);

	req->append_output_body_nocopy(body, len);
	*http_task->get_req() = std::move(*req);

	cout << "body=>" << (char*)body << endl;
	/* also, limit the remote webserver response size. */
	http_task->get_resp()->set_size_limit(200 * 1024 * 1024);

	*series << http_task;
}

void sig_handler(int signo) { }

int main(int argc, char* argv[])
{
	unsigned short port;

	port = 8222;// atoi(argv[1]);
	signal(SIGINT, sig_handler);
	cout << "start port=" << port << endl;
	struct WFServerParams params = HTTP_SERVER_PARAMS_DEFAULT;
	/* for safety, limit request size to 8MB. */
	params.request_size_limit = 200 * 1024 * 1024;

	WFHttpServer server(&params, process);
	
	
	cout << "server run" << endl;
	if (server.start(port) == 0)
	{
#ifndef _WIN32
		cout << "_WIN32 run" << endl;
		pause();
#else
		cout << "_WIN32 not" << endl;
		getchar();
#endif
		server.stop();
	}
	else
	{
		perror("Cannot start server");
		exit(1);
	}

	return 0;
}

