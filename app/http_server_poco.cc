#include "Poco/URI.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/TCPServer.h"
#include "Poco/Net/TCPServerConnection.h"
#include "Poco/Net/TCPServerConnectionFactory.h"
#include "Poco/Net/TCPServerParams.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/DatagramSocket.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/Exception.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Net/HTMLForm.h"

#include <nlohmann/json.hpp>

#include <string>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <ctime>

using namespace Poco::Net;
using namespace Poco::Util;

std::string parse_form(Poco::Net::HTTPServerRequest& request, const std::string& key) {
    std::string name;
    std::string value;
    Poco::Net::HTMLForm form( request );

    Poco::Net::NameValueCollection::ConstIterator i = form.begin();

    while(i!=form.end()){
        name=i->first;
        value=i->second;
        std::cout << name << "=" << value << std::endl << std::flush;
        if(name == key)
            return value;
        ++i;
    }
    return "";
}

std::string get_post_data(Poco::Net::HTTPServerRequest& req) {
    auto & stream = req.stream();
    const size_t len = req.getContentLength();
    std::string buffer(len, 0);
    stream.read((char*) buffer.data(), len);
    return buffer;
}

void pingHandler(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
    Poco::Util::Application& app = Poco::Util::Application::instance();
    app.logger().information("Request from " + request.clientAddress().toString());

    response.setStatus(HTTPResponse::HTTP_OK);
    response.setChunkedTransferEncoding(false);
    response.setContentType("application/json");

    std::ostream& out = response.send();
    out << "pong";
    out.flush();
}

void helloWorldHandler(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
    Poco::Util::Application& app = Poco::Util::Application::instance();
    app.logger().information("Request from " + request.clientAddress().toString());

    response.setStatus(HTTPResponse::HTTP_OK);
    response.setChunkedTransferEncoding(false);
    response.setContentType("application/json");

    std::cout << "Parsing request params: " << std::endl;
    std::string requestParam = parse_form(request, "name");

    std::ostream& out = response.send();

    std::srand(std::time(nullptr)); // use current time as seed for random generator
    int Id = std::rand();

    std::string nameVal;
    if(requestParam.size() == 0) {
        nameVal = "Stranger";
    } else {
        nameVal = requestParam;
    }

    nlohmann::json res;
    res["id"] = Id;
    res["name"] = std::string("Hello ") + nameVal;

    out << res.dump();

    out.flush();
}

void sumHandler(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
    Poco::Util::Application& app = Poco::Util::Application::instance();
    app.logger().information("Request from " + request.clientAddress().toString());

    response.setStatus(HTTPResponse::HTTP_OK);
    response.setChunkedTransferEncoding(false);
    response.setContentType("application/json");

    std::cout << "Parsing request params: " << std::endl;
    std::string requestParam = parse_form(request, "name");

    std::ostream& out = response.send();

    std::string post_data = get_post_data(request);
    std::cout << "Http post: " << post_data << std::endl;

    try
    {
        nlohmann::json js = nlohmann::json::parse(post_data);
        long x = js.at("x").get<long>();
        long y = js.at("y").get<long>();
        long sum = x + y;
        nlohmann::json res;
        res["sum"] = sum;
        std::cout << res.dump();
        out << res.dump();
        out.flush();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

class HttpRequestHandlerPoco : public HTTPRequestHandler {
public:
    virtual void handleRequest(HTTPServerRequest& req, HTTPServerResponse& resp)
    {
        Poco::URI uri = Poco::URI(req.getURI());
        std::string path = uri.getPath();
        std::string method = req.getMethod();

        std::cout << "<h1>Hello world!</h1>"
            << "<p>Host: " << req.getHost() << "</p>"
            << "<p>Method: " << method << "</p>"
            << "<p>URI: " << req.getURI() << "</p>"
            << "<p>Path: " << path << "</p>"
            << std::endl;

        if(path == "/ping") {
            if(method == "GET") {
                pingHandler(req, resp);
            }
        }

        if(path == "/hello-world") {
            if(method == "GET") {
                helloWorldHandler(req, resp);
            }
        }

        if(path == "/sum") {
            if(method == "POST") {
                sumHandler(req, resp);
            }
        }
    }
};

class HttpRequestHandlerPocoFactory : public HTTPRequestHandlerFactory {
public:
    virtual HTTPRequestHandler* createRequestHandler(const HTTPServerRequest&)
    {
        return new HttpRequestHandlerPoco;
    }
};

class HttpServerPoco : public ServerApplication {
protected:
    int main(const std::vector<std::string>& args)
    {
        HTTPServerParams* pParams = new HTTPServerParams;
        pParams->setKeepAlive(false);
        pParams->setMaxThreads(10);

        HTTPServer server(new HttpRequestHandlerPocoFactory, ServerSocket(8080), pParams);
        server.start();
        std::cout << std::endl << "Starting http server by Poco" << std::endl;
        waitForTerminationRequest(); // wait for CTRL-C or kill
        std::cout << std::endl << "Closing" << std::endl;
        server.stop();
        return Application::EXIT_OK;
    }
};

int main(int argc, char** argv)
{
    HttpServerPoco app;
    return app.run(argc, argv);
}

