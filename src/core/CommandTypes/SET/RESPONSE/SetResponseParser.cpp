#include "core/CommandTypes/SET/RESPONSE/SetResponseParser.h"

#include "core/enums.h"
#include <variant>

auto SetResponseParser::verify(const std::vector<uint8_t> &data) -> VerifyFrameResponse
{
    VerifyFrameResponse response;
    response.valid = true;

    auto serviceType = static_cast<ServiceType>(data[1]);
    auto invokeInfo = ParseHeader::decodeInvokeField(data[2]);

    auto headerResult = ParseHeader::parse_header(serviceType, invokeInfo.priority, invokeInfo.serviceClass);

    if (std::holds_alternative<ValidationError>(headerResult))
    {
        response.valid = false;
        response.errors.push_back(std::get<ValidationError>(headerResult));
        return response;
    }
    else
    {
        response.fields.push_back(std::get<ParsedField>(headerResult));
    }

    constexpr size_t minimumSize = 4;

    if (data.size() < minimumSize)
    {
        response.valid = false;

        response.errors.push_back({1, "SET-RESPONSE-NORMAL incompleto.", ""});

        return response;
    }

    auto result = parseDataAccessResult(data[3]);

    if (std::holds_alternative<ValidationError>(result))
    {
        response.valid = false;

        response.errors.push_back(std::get<ValidationError>(result));

        return response;
    }

    response.fields.push_back(std::get<ParsedField>(result));

    return response;
}

auto SetResponseParser::parseDataAccessResult(uint8_t value) -> std::variant<ParsedField, ValidationError>
{
    switch (value)
    {
    case 0:
        return ParsedField{"Data-Access-Result", 3, 1, "0", "Success"};

    case 1:
        return ParsedField{"Data-Access-Result", 3, 1, "1", "Hardware Fault"};

    case 2:
        return ParsedField{"Data-Access-Result", 3, 1, "2", "Temporary Failure"};

    case 3:
        return ParsedField{"Data-Access-Result", 3, 1, "3", "Read/Write Denied"};

    case 4:
        return ParsedField{"Data-Access-Result", 3, 1, "4", "Object Undefined"};

    case 9:
        return ParsedField{"Data-Access-Result", 3, 1, "9", "Object Class Inconsistent"};

    case 11:
        return ParsedField{"Data-Access-Result", 3, 1, "11", "Object Unavailable"};

    case 12:
        return ParsedField{"Data-Access-Result", 3, 1, "12", "Type Unmatched"};

    case 13:
        return ParsedField{"Data-Access-Result", 3, 1, "13", "Scope of Acess Violated"};

    case 14:
        return ParsedField{"Data-Access-Result", 3, 1, "14", "Data Block Unavailable"};

    case 15:
        return ParsedField{"Data-Access-Result", 3, 1, "15", "Long get Aborted"};

    case 16:
        return ParsedField{"Data-Access-Result", 3, 1, "16", "No Long get in Progress"};

    case 17:
        return ParsedField{"Data-Access-Result", 3, 1, "17", "Long Set Aborted"};

    case 18:
        return ParsedField{"Data-Access-Result", 3, 1, "18", "No Long Set in Progress"};

    case 19:
        return ParsedField{"Data-Access-Result", 3, 1, "19", "Data Block Number invalid"};

    case 250:
        return ParsedField{"Data-Access-Result", 3, 1, "250", "Other Reason"};

    default:
        return ValidationError{2, "Data-Access-Result desconhecido.", ""};
    }
}