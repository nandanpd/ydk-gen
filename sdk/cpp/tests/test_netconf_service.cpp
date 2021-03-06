/*  ----------------------------------------------------------------
 Copyright 2016 Cisco Systems

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 ------------------------------------------------------------------*/

#include <string.h>
#include <iostream>

#include <ydk/netconf_provider.hpp>
#include <ydk/netconf_service.hpp>
#include <ydk_ydktest/ydktest_sanity.hpp>
#include <ydk_ydktest/openconfig_bgp.hpp>
#include <ydk_ydktest/openconfig_interfaces.hpp>
#include <ydk/types.hpp>

#include <spdlog/spdlog.h>

#include "config.hpp"
#include "catch.hpp"

using namespace ydk;
using namespace ydktest;
using namespace std;

void print_data_node(shared_ptr<ydk::path::DataNode> dn);
void print_entity(shared_ptr<ydk::Entity> entity, ydk::path::RootSchemaNode& root);

// cancel_commit -- issues in netsim
//TEST_CASE("cancel_commit")
//{
//    // session
//    path::Repository repo{TEST_HOME};
//    NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
//    NetconfService ns{};
//
//    auto reply = ns.cancel_commit(session);
//    REQUIRE(reply);
//}

// close_session
TEST_CASE("close_session")
{
    // session
    path::Repository repo{TEST_HOME};
    NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
    NetconfService ns{};

    auto reply = ns.close_session(provider);
    REQUIRE(reply);
}

// commit
TEST_CASE("commit")
{
    // session
    path::Repository repo{TEST_HOME};
    NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
    NetconfService ns{};

    auto reply = ns.commit(provider);
    REQUIRE(reply);
}

// copy_config
TEST_CASE("copy_config")
{
    // session
    path::Repository repo{TEST_HOME};
    NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
    NetconfService ns{};

    DataStore target = DataStore::candidate;
    DataStore source = DataStore::running;

    auto reply = ns.copy_config(provider, target, source);
    REQUIRE(reply);
}

// delete_config -- issues in netsim
TEST_CASE("delete_config")
{
    // session
    path::Repository repo{TEST_HOME};
    NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
    NetconfService ns{};

    DataStore target = DataStore::url;

//    auto reply = ns.delete_config(session, target, "http://test");
    CHECK_THROWS_AS(ns.delete_config(provider, target, "http://test"), YError);
}

// discard_changes
TEST_CASE("discard_changes")
{
    // session
    path::Repository repo{TEST_HOME};
    NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
    NetconfService ns{};

    auto reply = ns.discard_changes(provider);
    REQUIRE(reply);
}

TEST_CASE("get_edit_copy_config")
{
    path::Repository repo{TEST_HOME};
    NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
    NetconfService ns{};

    DataStore target = DataStore::candidate;
    DataStore source = DataStore::running;

    // Build filter
    openconfig_interfaces::Interfaces interfaces_filter{};
    openconfig_bgp::Bgp bgp_filter{};
    vector<ydk::Entity*> filter_list{};
    filter_list.push_back(&interfaces_filter);
    filter_list.push_back(&bgp_filter);

    // Read running config
    auto get_config_list = ns.get_config(provider, source, filter_list);

    vector<Entity*> copy_config_list{};
    for (auto ent : get_config_list) {
    	//print_entity(ent, provider.get_session().get_root_schema());
        copy_config_list.push_back(ent.get());
    }

    // Copy config to candidate
    auto result = ns.copy_config(provider, target, copy_config_list);
    REQUIRE(result);

    // Discard changes
    result = ns.discard_changes(provider);
    REQUIRE(result);
}

// edit_config, get_config
TEST_CASE("edit_config")
{
    // session
    path::Repository repo{TEST_HOME};
    NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
    NetconfService ns{};

    DataStore target = DataStore::candidate;
    DataStore source = DataStore::candidate;
    openconfig_bgp::Bgp filter = {};
    openconfig_bgp::Bgp bgp = {};
    bgp.global->config->as = 6500;

    auto reply = ns.edit_config(provider, target, bgp);
    REQUIRE(reply);

    auto data = ns.get_config(provider, source, filter);
    REQUIRE(data);

    auto data_ptr = dynamic_cast<openconfig_bgp::Bgp*>(data.get());
    REQUIRE(data_ptr != nullptr);
    REQUIRE(data_ptr->global->config->as == bgp.global->config->as);

    reply = ns.discard_changes(provider);
    REQUIRE(reply);
}

TEST_CASE("edit_multiple_config")
{
    // session
    path::Repository repo{TEST_HOME};
    NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
    NetconfService ns{};

    DataStore target = DataStore::candidate;
    DataStore source = DataStore::candidate;

    // Create 'native' configuration
    ydktest::ydktest_sanity::Native native{};
    native.hostname = "My Host";
    native.version = "0.1.2";

    // Set the Global AS
    openconfig_bgp::Bgp bgp{};
    bgp.global->config->as = 65051;
    bgp.global->config->router_id = "10.20.30.40";

    // Create entity list
    vector<ydk::Entity*> edit_list{};
    edit_list.push_back(&native);
    edit_list.push_back(&bgp);

    auto reply = ns.edit_config(provider, target, edit_list);
    REQUIRE(reply);

    // Build filter
    ydktest::ydktest_sanity::Native native_filter{};
    openconfig_bgp::Bgp bgp_filter{};
    vector<ydk::Entity*> filter_list{};
    filter_list.push_back(&native_filter);
    filter_list.push_back(&bgp_filter);

    // Read current configuration and print it
    auto read_list = ns.get_config(provider, source, filter_list);
    REQUIRE(read_list.size() == 2);
    for (auto item : read_list) {
        string path = item->get_segment_path();
        if (path.find("bgp") != string::npos) {
            auto data_ptr = dynamic_cast<openconfig_bgp::Bgp*>(item.get());
            REQUIRE(data_ptr != nullptr);
            REQUIRE(data_ptr->global->config->as == bgp.global->config->as);
            REQUIRE(data_ptr->global->config->router_id == bgp.global->config->router_id);
        }
        if (path.find("native") != string::npos) {
            auto data_ptr = dynamic_cast<ydktest::ydktest_sanity::Native*>(item.get());
            REQUIRE(data_ptr != nullptr);
            REQUIRE(data_ptr->hostname == native.hostname);
            REQUIRE(data_ptr->version == native.version);
        }
    }

    // Discard config changes
    reply = ns.discard_changes(provider);
    REQUIRE(reply);
}

// get
TEST_CASE("get")
{
    // session
    path::Repository repo{TEST_HOME};
    NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
    NetconfService ns{};

    openconfig_bgp::Bgp filter = {};

    auto reply = ns.get(provider, filter);
    REQUIRE(reply);
}

// kill_session
TEST_CASE("kill_session")
{
    // session
    path::Repository repo{TEST_HOME};
    NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
    NetconfService ns{};

    int session_id = 3;

//    auto reply = ns.kill_session(session, session_id);
    CHECK_THROWS_AS(ns.kill_session(provider, session_id), YError);
}

// lock, unlock
TEST_CASE("lock")
{
    // session
    path::Repository repo{TEST_HOME};
    NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
    NetconfService ns{};

    DataStore target = DataStore::candidate;

    auto reply = ns.lock(provider, target);
    REQUIRE(reply);

    reply = ns.unlock(provider, target);
    REQUIRE(reply);
}

// validate
TEST_CASE("validate")
{
    // session
    path::Repository repo{TEST_HOME};
    NetconfServiceProvider provider{repo, "127.0.0.1", "admin", "admin", 12022};
    NetconfService ns{};

    DataStore source = DataStore::candidate;

    auto reply = ns.validate(provider, source);
    REQUIRE(reply);
}

// TODO
//TEST_CASE("read_all_rpc")
//{
//    //ydk::path::NetconfSession session{"10.30.110.86", "admin", "admin"};	// XR
//
//    ydk::path::NetconfSession session{"127.0.0.1", "admin", "admin",  12022};
//    ydk::path::RootSchemaNode& schema = session.get_root_schema();
//
//    std::shared_ptr<ydk::path::Rpc> read_rpc { schema.create_rpc("ietf-netconf:get-config") };
//    read_rpc->get_input_node().create_datanode("source/running");
//
//    auto read_result = (*read_rpc)(session);
//    REQUIRE(read_result != nullptr);
//
//    // Print config
//    vector<shared_ptr<ydk::path::DataNode>> data_nodes = read_result->get_children();
//    for (auto dn : data_nodes) {
//    	print_data_node(dn);
//    }
//}
