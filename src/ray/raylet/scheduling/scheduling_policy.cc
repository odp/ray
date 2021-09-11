// Copyright 2021 The Ray Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "ray/raylet/scheduling/scheduling_policy.h"

#include <algorithm>

namespace ray {

namespace raylet_scheduling_policy {

struct NodeInfo {
  bool is_feasible = false;
  bool is_available = false;
  float critical_resource_utilization = 1.0f;
  friend std::ostream &operator<<(std::ostream &os, const NodeInfo &info) {
    return os << (info.is_feasible ? "feasible" : "!feasible") << ","
              << (info.is_available ? "available" : "!available") << ","
              << "critical: " << info.critical_resource_utilization;
  }
};

NodeInfo GetNodeInfo(const Node &node, const ResourceRequest &resource_request) {
  NodeInfo info;
  info.is_feasible = node.GetLocalView().IsFeasible(resource_request);
  if (!info.is_feasible) {
    return info;
  }
  info.is_available = node.GetLocalView().IsAvailable(resource_request, true);
  if (!info.is_available) {
    return info;
  }
  // TODO: consider taking resource_request into account when computing
  // CalculateCriticalResourceUtilization.
  info.critical_resource_utilization =
      node.GetLocalView().CalculateCriticalResourceUtilization();
  return info;
}

int64_t NewHybridPolicy(const ResourceRequest &resource_request,
                        const int64_t local_node_id,
                        const absl::flat_hash_map<int64_t, Node> &nodes,
                        float spread_threshold, bool force_spillback,
                        bool require_available) {
  // Similarly to NewHybridPolicy, prefer scheduling on local node,
  // perform packing in case of low utilization.
  // Perform weighted random scheduling otherwise.

  const auto local_node_it = nodes.find(local_node_id);
  RAY_CHECK(local_node_it != nodes.end());
  const auto &local_node = local_node_it->second;
  const auto local_info = GetNodeInfo(local_node, resource_request);
  RAY_LOG(DEBUG) << "Local node: " << local_node_id << " " << local_info;

  if (!force_spillback && local_info.is_feasible && local_info.is_available &&
      local_info.critical_resource_utilization < spread_threshold) {
    return local_node_id;
  }

  int64_t feasible_node_id = -1;
  if (local_info.is_feasible && !force_spillback) {
    RAY_LOG(DEBUG) << "feasible id: " << feasible_node_id;
    feasible_node_id = local_node_id;
  }

  std::vector<std::pair<float, int64_t>> ut_node_id;
  ut_node_id.reserve(nodes.size());
  std::vector<int64_t> ids;
  ids.reserve(nodes.size());
  for (const auto &pair : nodes) {
    if (pair.first != local_node_id) {
      ids.push_back(pair.first);
    }
  }
  std::sort(ids.begin(), ids.end());

  for (auto id_it = ids.begin(); id_it != ids.end(); ++id_it) {
    const auto node_id = *id_it;
    const auto node_it = nodes.find(node_id);
    RAY_CHECK(node_it != nodes.end());
    const auto &node = node_it->second;
    const auto info = GetNodeInfo(node, resource_request);

    RAY_LOG(DEBUG) << "node " << node_id << " " << info
                   << " spread_threshold=" << spread_threshold;

    if (!info.is_feasible) {
      continue;
    }
    if (feasible_node_id == -1) {
      RAY_LOG(DEBUG) << "feasible id: " << feasible_node_id;
      feasible_node_id = node_id;
    }
    if (!info.is_available) {
      continue;
    }
    if (info.critical_resource_utilization < spread_threshold) {
      return node_id;
    }
    ut_node_id.push_back({1.0f - info.critical_resource_utilization, node_id});
  }

  if (ut_node_id.empty()) {
    if (require_available) {
      return -1;
    }
    return feasible_node_id;
  }
  std::sort(ut_node_id.begin(), ut_node_id.end());

  float sum = 0;
  for (auto it = ut_node_id.begin(); it != ut_node_id.end(); ++it) {
    const float tmp = it->first;
    it->first = sum;
    sum += tmp;
  }
  static thread_local std::default_random_engine gen;
  std::uniform_real_distribution<double> distribution(0, sum);
  const float w = distribution(gen);
  auto lb = std::lower_bound(ut_node_id.begin(), ut_node_id.end(),
                             std::make_pair(w, int64_t()));
  if (lb == ut_node_id.end()) {
    // Just in case.
    --lb;
  }

  const int64_t random_node_id = lb->second;
  RAY_LOG(DEBUG) << "random_node_id=" << random_node_id << " w:" << w << " max:" << sum;
  return random_node_id;
}

int64_t HybridPolicy(const ResourceRequest &resource_request, const int64_t local_node_id,
                     const absl::flat_hash_map<int64_t, Node> &nodes,
                     float spread_threshold, bool force_spillback,
                     bool require_available) {
  return NewHybridPolicy(resource_request, local_node_id, nodes, spread_threshold,
                         force_spillback, require_available);
  // Step 1: Generate the traversal order. We guarantee that the first node is local, to
  // encourage local scheduling. The rest of the traversal order should be globally
  // consistent, to encourage using "warm" workers.
  std::vector<int64_t> round;
  {
    // Make sure the local node is at the front of the list so that 1. It's first in
    // traversal order. 2. It's easy to avoid sorting it.
    round.push_back(local_node_id);
    for (const auto &pair : nodes) {
      if (pair.first != local_node_id) {
        round.push_back(pair.first);
      }
    }
    std::sort(round.begin() + 1, round.end());
  }

  int64_t best_node_id = -1;
  float best_utilization_score = INFINITY;
  bool best_is_available = false;

  // Step 2: Perform the round robin.
  auto round_it = round.begin();
  if (force_spillback) {
    // The first node will always be the local node. If we want to spillback, we can
    // just never consider scheduling locally.
    round_it++;
  }
  for (; round_it != round.end(); round_it++) {
    const auto &node_id = *round_it;
    const auto &it = nodes.find(node_id);
    RAY_CHECK(it != nodes.end());
    const auto &node = it->second;
    if (!node.GetLocalView().IsFeasible(resource_request)) {
      continue;
    }

    bool ignore_pull_manager_at_capacity = false;
    if (node_id == local_node_id) {
      // It's okay if the local node's pull manager is at
      // capacity because we will eventually spill the task
      // back from the waiting queue if its args cannot be
      // pulled.
      ignore_pull_manager_at_capacity = true;
    }
    bool is_available = node.GetLocalView().IsAvailable(resource_request,
                                                        ignore_pull_manager_at_capacity);
    RAY_LOG(DEBUG) << "Node " << node_id << " is "
                   << (is_available ? "available" : "not available");
    float critical_resource_utilization =
        node.GetLocalView().CalculateCriticalResourceUtilization();
    if (critical_resource_utilization < spread_threshold) {
      critical_resource_utilization = 0;
    }

    bool update_best_node = false;

    if (is_available) {
      // Always prioritize available nodes over nodes where the task must be queued
      // first.
      if (!best_is_available) {
        update_best_node = true;
      } else if (critical_resource_utilization < best_utilization_score) {
        // Break ties between available nodes by their critical resource utilization.
        update_best_node = true;
      }
    } else if (!best_is_available &&
               critical_resource_utilization < best_utilization_score &&
               !require_available) {
      // Pick the best feasible node by critical resource utilization.
      update_best_node = true;
    }

    if (update_best_node) {
      best_node_id = node_id;
      best_utilization_score = critical_resource_utilization;
      best_is_available = is_available;
    }
  }

  return best_node_id;
}

}  // namespace raylet_scheduling_policy

}  // namespace ray
