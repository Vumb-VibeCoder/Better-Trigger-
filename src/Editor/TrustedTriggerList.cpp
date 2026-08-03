#include "TrustedTriggerList.hpp"
#include "../Knowledge/TriggerSchema.hpp"
#include <sstream>

namespace ta {

namespace {
    std::string paramTypeName(ParamType t) {
        switch (t) {
            case ParamType::Float:   return "float";
            case ParamType::Int:     return "int";
            case ParamType::Bool:    return "bool (0 hoac 1)";
            case ParamType::Easing:  return "easing id (0-18, xem bang phia duoi)";
            case ParamType::GroupId: return "group id (int, 0-9999)";
            case ParamType::ChanceList:
                return "MANG cac object {\"groupId\":<int>,\"chance\":<int 0-100>}, "
                       "so luong phan tu tuy y theo yeu cau nguoi dung";
        }
        return "float";
    }
}

std::string buildTrustedTriggerListForPrompt() {
    std::ostringstream out;
    out << "Danh sach trigger DUOC XAC NHAN DUNG (chi duoc chon id trong danh sach nay, "
           "KHONG duoc dung id nao khac ngoai danh sach du ban co the doan duoc):\n\n";

    for (auto& s : triggerSchemas()) {
        out << "- " << s.displayName << " (id=" << s.gdObjectId << ")";
        if (!s.aliases.empty()) {
            out << " [tu khoa: ";
            bool first = true;
            for (auto& a : s.aliases) {
                if (!first) out << ", ";
                out << a;
                first = false;
            }
            out << "]";
        }
        out << "\n";

        for (auto& p : s.params) {
            if (p.type == ParamType::ChanceList) {
                out << "    * " << p.name << ": " << paramTypeName(p.type) << "\n";
            } else {
                out << "    * " << p.name << ": " << paramTypeName(p.type)
                    << ", khoang [" << p.minValue << ".." << p.maxValue << "]"
                    << ", mac dinh=" << p.defaultValue << "\n";
            }
        }
    }

    out << "\nBang Easing ID (dung cho tham so \"easing\"):\n"
           "0=None, 1=EaseInOut, 2=EaseIn, 3=EaseOut, 4=ElasticInOut, 5=ElasticIn, "
           "6=ElasticOut, 7=BounceInOut, 8=BounceIn, 9=BounceOut, 10=ExponentialInOut, "
           "11=ExponentialIn, 12=ExponentialOut, 13=SineInOut, 14=SineIn, 15=SineOut, "
           "16=BackInOut, 17=BackIn, 18=BackOut.\n";

    out << "\nQUAN TRONG ve don vi moveX/moveY: tra ve theo DON VI HIEN THI giong nhu "
           "nguoi choi go tay vao popup Move Trigger trong editor (vi du nguoi dung noi "
           "\"di chuyen sang phai 10 o luoi\" thi moveX = 100, vi 1 o luoi = 10 don vi hien "
           "thi). KHONG tra ve theo pixel thuc te - he thong se tu quy doi sang pixel.\n";

    out << "\nQUAN TRONG - phan biet 2 khai niem GROUP KHAC NHAU, KHONG duoc nham lan:\n"
           "1) \"targetGroup\": group cua OBJECT/BLOCK BI TRIGGER TAC DONG (vd Move Trigger "
           "di chuyen group nao, Color Trigger doi mau group nao). Chi dien khi trigger co "
           "tac dong len mot group cu extreme va nguoi dung co neu ro group do.\n"
           "2) \"tag\": Group ID cua CHINH BAN THAN cai trigger nay (giong nguoi dung tu tay "
           "gan Group ID cho trigger trong Edit Object), de TRIGGER KHAC (vd Spawn Trigger, "
           "Touch Trigger) co the goi/kich hoat den no. CHI dien \"tag\" khi nguoi dung yeu "
           "cau ro rang (vi du: \"tao move trigger voi tag la 50\", \"gan group id 20 cho "
           "trigger nay de spawn trigger goi toi\").\n"
           "TUYET DOI KHONG tu suy ra \"tag\" tu \"targetGroup\" hay nguoc lai - hai gia tri "
           "nay doc lap hoan toan va co the khac nhau. Neu nguoi dung chi noi \"tag trigger "
           "nay\" ma KHONG noi gi ve group bi tac dong, CHI dien \"tag\", KHONG duoc tu dien "
           "them \"targetGroup\".\n";

    out << "\nQUAN TRONG - 3 tham so CHUNG CHO MOI TRIGGER (khong lap lai trong "
           "danh sach tren nhung trigger nao CUNG dung duoc, giong 3 checkbox "
           "that trong popup GD):\n"
           "- \"touchTriggered\" (bool): trigger CHI kich hoat khi nguoi choi "
           "cham vao no, thay vi tu kich hoat khi qua vach.\n"
           "- \"spawnTriggered\" (bool): trigger KHONG tu kich hoat, chi chay "
           "khi duoc mot trigger khac (Spawn/Touch/Count/...) goi toi qua tag.\n"
           "- \"multiTriggered\" (bool): cho phep trigger kich hoat NHIEU LAN "
           "thay vi chi 1 lan.\n"
           "CHI dien 3 tham so nay khi nguoi dung yeu cau ro rang (vd \"cham vao "
           "moi kich hoat\", \"bat spawn triggered\", \"cho phep kich hoat nhieu "
           "lan\") - KHONG tu them mac dinh.\n";

    out << "\nQUAN TRONG ve \"chanceList\" (random_trigger / advanced_random_trigger / "
           "sequence_trigger): day la MOT MANG, moi phan tu la {\"groupId\":<int>,"
           "\"chance\":<int 0-100>} - MOI group duoc kich hoat co trong so (chance) rieng. "
           "TUYET DOI KHONG dung tham so \"targetGroup\" don le cho 3 loai trigger nay - "
           "phai dua toan bo danh sach group vao \"chanceList\".\n";

    out << "\nNeu nguoi dung yeu cau mot trigger KHONG nam trong danh sach tren, hoac mot "
           "tham so KHONG duoc liet ke cho dung trigger do, TRA VE mang \"objects\" RONG ([]) "
           "hoac BO QUA tham so do - TUYET DOI KHONG duoc bia dat id/ten tham so moi du co ve "
           "hop ly hay giong voi trigger khac.";

    return out.str();
}

} // namespace ta