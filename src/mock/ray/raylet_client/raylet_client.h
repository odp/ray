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

namespace ray {

class MockPinObjectsInterface : public PinObjectsInterface {
 public:
  MOCK_METHOD(void, PinObjectIDs,
              (const rpc::Address &caller_address,
               const std::vector<ObjectID> &object_ids,
               const ray::rpc::ClientCallback<ray::rpc::PinObjectIDsReply> &callback),
              (override));
};

}  // namespace ray

namespace ray {

class MockWorkerLeaseInterface : public WorkerLeaseInterface {
 public:
  MOCK_METHOD(
      void, RequestWorkerLease,
      (const ray::TaskSpecification &resource_spec,
       const ray::rpc::ClientCallback<ray::rpc::RequestWorkerLeaseReply> &callback,
       const int64_t backlog_size),
      (override));
  MOCK_METHOD(
      void, RequestWorkerLease,
      (const rpc::TaskSpec &task_spec,
       const ray::rpc::ClientCallback<ray::rpc::RequestWorkerLeaseReply> &callback,
       const int64_t backlog_size),
      (override));
  MOCK_METHOD(ray::Status, ReturnWorker,
              (int worker_port, const WorkerID &worker_id, bool disconnect_worker),
              (override));
  MOCK_METHOD(void, ReleaseUnusedWorkers,
              (const std::vector<WorkerID> &workers_in_use,
               const rpc::ClientCallback<rpc::ReleaseUnusedWorkersReply> &callback),
              (override));
  MOCK_METHOD(void, CancelWorkerLease,
              (const TaskID &task_id,
               const rpc::ClientCallback<rpc::CancelWorkerLeaseReply> &callback),
              (override));
};

}  // namespace ray

namespace ray {

class MockResourceReserveInterface : public ResourceReserveInterface {
 public:
  MOCK_METHOD(
      void, PrepareBundleResources,
      (const BundleSpecification &bundle_spec,
       const ray::rpc::ClientCallback<ray::rpc::PrepareBundleResourcesReply> &callback),
      (override));
  MOCK_METHOD(
      void, CommitBundleResources,
      (const BundleSpecification &bundle_spec,
       const ray::rpc::ClientCallback<ray::rpc::CommitBundleResourcesReply> &callback),
      (override));
  MOCK_METHOD(
      void, CancelResourceReserve,
      (const BundleSpecification &bundle_spec,
       const ray::rpc::ClientCallback<ray::rpc::CancelResourceReserveReply> &callback),
      (override));
  MOCK_METHOD(void, ReleaseUnusedBundles,
              (const std::vector<rpc::Bundle> &bundles_in_use,
               const rpc::ClientCallback<rpc::ReleaseUnusedBundlesReply> &callback),
              (override));
};

}  // namespace ray

namespace ray {

class MockDependencyWaiterInterface : public DependencyWaiterInterface {
 public:
  MOCK_METHOD(ray::Status, WaitForDirectActorCallArgs,
              (const std::vector<rpc::ObjectReference> &references, int64_t tag),
              (override));
};

}  // namespace ray

namespace ray {

class MockResourceTrackingInterface : public ResourceTrackingInterface {
 public:
  MOCK_METHOD(void, UpdateResourceUsage,
              (std::string & serialized_resource_usage_batch,
               const rpc::ClientCallback<rpc::UpdateResourceUsageReply> &callback),
              (override));
  MOCK_METHOD(void, RequestResourceReport,
              (const rpc::ClientCallback<rpc::RequestResourceReportReply> &callback),
              (override));
};

}  // namespace ray

namespace ray {

class MockRayletClientInterface : public RayletClientInterface {
 public:
  MOCK_METHOD(ray::Status, WaitForDirectActorCallArgs,
              (const std::vector<rpc::ObjectReference> &references, int64_t tag),
              (override));
  MOCK_METHOD(
      void, RequestWorkerLease,
      (const ray::TaskSpecification &resource_spec,
       const ray::rpc::ClientCallback<ray::rpc::RequestWorkerLeaseReply> &callback,
       const int64_t backlog_size),
      (override));
  MOCK_METHOD(
      void, RequestWorkerLease,
      (const rpc::TaskSpec &resource_spec,
       const ray::rpc::ClientCallback<ray::rpc::RequestWorkerLeaseReply> &callback,
       const int64_t backlog_size),
      (override));

  MOCK_METHOD(ray::Status, ReturnWorker,
              (int worker_port, const WorkerID &worker_id, bool disconnect_worker),
              (override));
  MOCK_METHOD(void, ReleaseUnusedWorkers,
              (const std::vector<WorkerID> &workers_in_use,
               const rpc::ClientCallback<rpc::ReleaseUnusedWorkersReply> &callback),
              (override));
  MOCK_METHOD(void, CancelWorkerLease,
              (const TaskID &task_id,
               const rpc::ClientCallback<rpc::CancelWorkerLeaseReply> &callback),
              (override));
  MOCK_METHOD(
      void, PrepareBundleResources,
      (const BundleSpecification &bundle_spec,
       const ray::rpc::ClientCallback<ray::rpc::PrepareBundleResourcesReply> &callback),
      (override));
  MOCK_METHOD(
      void, CommitBundleResources,
      (const BundleSpecification &bundle_spec,
       const ray::rpc::ClientCallback<ray::rpc::CommitBundleResourcesReply> &callback),
      (override));
  MOCK_METHOD(
      void, CancelResourceReserve,
      (const BundleSpecification &bundle_spec,
       const ray::rpc::ClientCallback<ray::rpc::CancelResourceReserveReply> &callback),
      (override));
  MOCK_METHOD(void, ReleaseUnusedBundles,
              (const std::vector<rpc::Bundle> &bundles_in_use,
               const rpc::ClientCallback<rpc::ReleaseUnusedBundlesReply> &callback),
              (override));
  MOCK_METHOD(void, PinObjectIDs,
              (const rpc::Address &caller_address,
               const std::vector<ObjectID> &object_ids,
               const ray::rpc::ClientCallback<ray::rpc::PinObjectIDsReply> &callback),
              (override));
  MOCK_METHOD(void, GetSystemConfig,
              (const rpc::ClientCallback<rpc::GetSystemConfigReply> &callback),
              (override));
  MOCK_METHOD(void, GetGcsServerAddress,
              (const rpc::ClientCallback<rpc::GetGcsServerAddressReply> &callback),
              (override));
  MOCK_METHOD(void, UpdateResourceUsage,
              (std::string & serialized_resource_usage_batch,
               const rpc::ClientCallback<rpc::UpdateResourceUsageReply> &callback),
              (override));
  MOCK_METHOD(void, RequestResourceReport,
              (const rpc::ClientCallback<rpc::RequestResourceReportReply> &callback),
              (override));
};

}  // namespace ray
