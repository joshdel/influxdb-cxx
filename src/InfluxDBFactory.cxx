///
/// \author Adam Wegrzynek <adam.wegrzynek@cern.ch>
///

#include "InfluxDBFactory.h"
#include <functional>
#include <string>
#include <memory>
#include "UriParser.h"
#include "HTTP.h"
#include "UDP.h"
#include "UnixSocket.h"

namespace influxdb 
{

std::unique_ptr<Transport> withUdpTransport(const http::url& uri) {
  return std::make_unique<transports::UDP>(uri.host, uri.port);
}

std::unique_ptr<Transport> withHttpTransport(const http::url& uri) {
  return std::make_unique<transports::HTTP>("http://" + uri.host + ":" + std::to_string(uri.port));
}

std::unique_ptr<Transport> withHttpsTransport(const http::url& uri) {
  return std::make_unique<transports::HTTP>("https://" + uri.host + ":" + std::to_string(uri.port));
}

std::unique_ptr<Transport> withUnixSocketTransport(const http::url& uri) {
  return std::make_unique<transports::UnixSocket>(uri.path);
}

std::unique_ptr<Transport> InfluxDBFactory::GetTransport(std::string url) {
  static const std::map<std::string, std::function<std::unique_ptr<Transport>(const http::url&)>> map = {
    {"udp", withUdpTransport},
    {"http", withHttpTransport},
    {"http", withHttpsTransport},
    {"unix", withUnixSocketTransport},
  };

  http::url parsedUrl = http::ParseHttpUrl(url);
  if (parsedUrl.protocol.empty()) {
    throw std::runtime_error("Ill-formed URI");
  }   

  auto iterator = map.find(parsedUrl.protocol);
  if (iterator == map.end()) {
    throw std::runtime_error("Unrecognized backend " + parsedUrl.protocol);
  }

  return iterator->second(parsedUrl);
}

std::unique_ptr<InfluxDB> InfluxDBFactory::Get(std::string url)
{
  return std::make_unique<InfluxDB>(InfluxDBFactory::GetTransport(url));
}

} // namespace influxdb