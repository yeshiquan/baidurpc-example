// Baidu RPC - A framework to host and access services throughout Baidu.
// Copyright (c) 2014 Baidu.com, Inc. All Rights Reserved
//
// If you have any problem, contact us:
//   Baidu Hi : group 1296497 
//   Email    : pbrpc@baidu.com
//   Wiki     : http://wiki.baidu.com/display/RPC/baidu-rpc

// A server to receive EchoRequest and send back EchoResponse.

#include <gflags/gflags.h>
#include <base/logging.h>
#include <baidu/rpc/server.h>
#include <baidu/rpc/nshead_service.h>
#include <mcpack/mcpack2pb.h>
#include "baidu/rpc/errno.pb.h"
#include "adder.pb.h"

DEFINE_int32(port, 8010, "TCP Port of this server");
DEFINE_int32(idle_timeout_s, -1, "Connection will be closed if there is no "
             "read/write operations during the last `idle_timeout_s'");
DEFINE_int32(max_concurrency, 0, "Limit of request processing in parallel");

// Adapt your own nshead-based protocol to use baidu-rpc 
class MyNsheadProtocol : public baidu::rpc::NsheadService {
public:
    void ProcessNsheadRequest(const baidu::rpc::Server&,
                              baidu::rpc::Controller* cntl,
                              const baidu::rpc::NsheadMessage& request,
                              baidu::rpc::NsheadMessage* response, 
                              baidu::rpc::NsheadClosure* done) {
        // This object helps you to call done->Run() in RAII style. If you need
        // to process the request asynchronously, pass done_guard.release().
        baidu::rpc::ClosureGuard done_guard(done);

        if (cntl->Failed()) {
            // NOTE: You can send back a response containing error information
            // back to client instead of closing the connection.
            cntl->CloseConnection("Close connection due to previous error");
            return;
        }

        RequestMessage req_msg;
        mcpack::MessageHandler req_handler = mcpack::find_message_handler("RequestMessage");
        if (!req_handler.parse_from_iobuf(&req_msg, request.body)) {
            cntl->SetFailed(2, "Fail to parse request message, "
                            "request_size=%d", request.body.length());
            return;
        }

        ResultMessage res_msg;
        mcpack::MessageHandler res_handler = mcpack::find_message_handler("ResultMessage");
        res_msg.set_sum(req_msg.a() + req_msg.b());
        const google::protobuf::Message* p = dynamic_cast<google::protobuf::Message *>(&res_msg);
        if (!res_handler.serialize_to_iobuf(*p, &response->body,
                                       mcpack::FORMAT_MCPACK_V2)) {
            cntl->CloseConnection("Fail to serialize TestMessage");
            return;
        }

        //*response = request; // Just echo the request to client
    }
};

int main(int argc, char* argv[]) {
    // Parse gflags. We recommend you to use gflags as well.
    google::ParseCommandLineFlags(&argc, &argv, true);

    baidu::rpc::Server server;
    baidu::rpc::ServerOptions options;
    options.nshead_service = new MyNsheadProtocol;
    options.idle_timeout_sec = FLAGS_idle_timeout_s;
    options.max_concurrency = FLAGS_max_concurrency;

    // Start the server.
    if (server.Start(FLAGS_port, &options) != 0) {
        LOG(ERROR) << "Fail to start EchoServer";
        return -1;
    }

    // Wait until Ctrl-C is pressed, then Stop() and Join() the server.
    server.RunUntilAskedToQuit();
    return 0;
}
