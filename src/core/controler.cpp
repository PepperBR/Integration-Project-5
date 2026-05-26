#include "core/controler.h"
Status Controler::HandleVerifyFrame(grpc::ServerContext *context, const os::VerifyFrameRequest *request, os::VerifyFrameResponse *response)
{
    // Implementation for handling frame verification
}

void Controler::HandleFrameToProto(std::shared_ptr<os::Frame> frame, os::Frame *proto_frame)
{
    // proto_frame->set_id(std::to_string(frame->getID()));
    // proto_frame->set_timestamp(frame->getTimestamp());
    // proto_frame->set_line_name(frame->getLineName());
    // proto_frame->set_model_name(frame->getModelName());

    // for (const auto &meter : frame->getMeters())
    // {
    //     auto proto_meter = proto_frame->add_meters();
    //     HandleMeterToProto(meter, proto_meter);
    // }
}