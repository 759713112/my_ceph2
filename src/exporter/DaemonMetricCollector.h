#pragma once

#include "common/admin_socket_client.h"
#include <map>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/json/object.hpp>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

struct pstat {
  unsigned long utime;
  unsigned long stime;
  unsigned long minflt;
  unsigned long majflt;
  unsigned long start_time;
  int num_threads;
  unsigned long vm_size;
  int resident_size;
};

class MetricsBuilder;
class OrderedMetricsBuilder;
class UnorderedMetricsBuilder;
class Metric;

typedef std::map<std::string, std::string> labels_t;

class DaemonMetricCollector {
public:
  void main();
  std::string get_metrics();

private:
  std::map<std::string, AdminSocketClient> clients;
  std::string metrics;
  std::mutex metrics_mutex;
  std::unique_ptr<MetricsBuilder> builder;
  void update_sockets();
  void request_loop(boost::asio::steady_timer &timer);

  void dump_asok_metrics();
  void dump_asok_metric(boost::json::object perf_info,
                        boost::json::value perf_values, std::string name,
                        labels_t labels);
  std::pair<labels_t, std::string>
  get_labels_and_metric_name(std::string daemon_name, std::string metric_name);
  void get_process_metrics(std::vector<std::pair<std::string, int>> daemon_pids);
  std::string asok_request(AdminSocketClient &asok, std::string command, std::string daemon_name);
};

class Metric {
private:
  struct metric_entry {
    labels_t labels;
    std::string value;
  };
  std::string name;
  std::string mtype;
  std::string description;
  std::vector<metric_entry> entries;

public:
  Metric(std::string name, std::string mtype, std::string description)
      : name(name), mtype(mtype), description(description) {}
  Metric(const Metric &) = default;
  Metric() = default;
  void add(labels_t labels, std::string value);
  std::string dump();
};

class MetricsBuilder {
public:
  virtual ~MetricsBuilder() = default;
  virtual std::string dump() = 0;
  virtual void add(std::string value, std::string name, std::string description,
                   std::string mtype, labels_t labels) = 0;

protected:
  std::string out;
};

class OrderedMetricsBuilder : public MetricsBuilder {
private:
  std::map<std::string, Metric> metrics;

public:
  std::string dump();
  void add(std::string value, std::string name, std::string description,
           std::string mtype, labels_t labels);
};

class UnorderedMetricsBuilder : public MetricsBuilder {
public:
  std::string dump();
  void add(std::string value, std::string name, std::string description,
           std::string mtype, labels_t labels);
};

DaemonMetricCollector &collector_instance();
