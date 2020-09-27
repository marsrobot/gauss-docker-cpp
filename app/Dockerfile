FROM ubuntu:20.04

RUN apt-get update
RUN apt-cache search nlohmann
RUN apt-get install -y gcc g++ openssl libssl-dev nlohmann-json3-dev libpoco-dev

EXPOSE 8080

WORKDIR /app

COPY . ./

RUN g++ -std=c++11 -o http_server_poco http_server_poco.cc -lPocoNet -lPocoUtil -lPocoFoundation

ENTRYPOINT ["/app/http_server_poco"]