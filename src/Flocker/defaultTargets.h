#ifndef DEFAULT_TARGETS_H_
#define DEFAULT_TARGETS_H_

struct match_mac_t
{
  const char* prefix;
  const char* name;
};

// default WiFi mac address prefixes
const match_mac_t wifiDefMacs[] = {{"70:1A:D5", "Avigilon Alta"}, {"00:40:8C", "Axis Communications AB"}, {"AC:CC:8E", "Axis Communications AB"}, {"B8:A4:4F", "Axis Communications AB"}, 
                       {"E8:27:25", "Axis Communications AB"}, {"1C:79:2D", "CHINA DRAGON TECHNOLOGY LIMITED"}, {"3C:3B:AD", "CHINA DRAGON TECHNOLOGY LIMITED"}, {"40:9C:A7", "CHINA DRAGON TECHNOLOGY LIMITED"}, 
                       {"54:AE:BC", "CHINA DRAGON TECHNOLOGY LIMITED"}, {"5C:8A:AE", "CHINA DRAGON TECHNOLOGY LIMITED"}, {"6C:05:D3", "CHINA DRAGON TECHNOLOGY LIMITED"}, {"A4:6B:40", "CHINA DRAGON TECHNOLOGY LIMITED"}, 
                       {"A8:4F:A4", "CHINA DRAGON TECHNOLOGY LIMITED"}, {"A8:A0:92", "CHINA DRAGON TECHNOLOGY LIMITED"}, {"B0:AC:82", "CHINA DRAGON TECHNOLOGY LIMITED"}, {"BC:2B:02", "CHINA DRAGON TECHNOLOGY LIMITED"}, 
                       {"C0:E3:50", "CHINA DRAGON TECHNOLOGY LIMITED"}, {"C8:26:E2", "CHINA DRAGON TECHNOLOGY LIMITED"}, {"C8:8A:D8", "CHINA DRAGON TECHNOLOGY LIMITED"}, {"00:7E:56", "China Dragon Technology Limited"}, 
                       {"04:39:26", "China Dragon Technology Limited"}, {"24:B7:2A", "China Dragon Technology Limited"}, {"3C:7A:AA", "China Dragon Technology Limited"}, {"40:AA:56", "China Dragon Technology Limited"}, 
                       {"44:EF:BF", "China Dragon Technology Limited"}, {"78:8A:86", "China Dragon Technology Limited"}, {"94:E0:D6", "China Dragon Technology Limited"}, {"A0:67:20", "China Dragon Technology Limited"}, 
                       {"A0:9D:C1", "China Dragon Technology Limited"}, {"A8:43:A4", "China Dragon Technology Limited"}, {"D0:A4:6F", "China Dragon Technology Limited"}, {"E0:51:D8", "China Dragon Technology Limited"}, 
                       {"E0:75:26", "China Dragon Technology Limited"}, {"00:13:56", "FLIR Radiation Inc"}, {"00:40:7F", "FLIR Systems"}, {"00:1B:D8", "FLIR Systems Inc"}, 
                       {"B4:1E:52", "Flock Safety"}, {"00:13:E2", "GeoVision Inc."}, {"44:B4:23", "HANWHA VISION VIETNAM COMPANY LIMITED"}, {"8C:1D:55", "Hanwha NxMD (Thailand) Co., Ltd."}, 
                       {"E4:30:22", "Hanwha Vision VietNam"}, {"00:10:BE", "MARCH NETWORKS CORPORATION"}, {"00:12:81", "March Networks S.p.A."}, {"48:05:60", "Meta Platforms, Inc."}, 
                       {"50:99:03", "Meta Platforms, Inc."}, {"78:C4:FA", "Meta Platforms, Inc."}, {"80:F3:EF", "Meta Platforms, Inc."}, {"84:57:F7", "Meta Platforms, Inc."}, 
                       {"88:25:08", "Meta Platforms, Inc."}, {"94:F9:29", "Meta Platforms, Inc."}, {"B4:17:A8", "Meta Platforms, Inc."}, {"C0:DD:8A", "Meta Platforms, Inc."}, 
                       {"CC:A1:74", "Meta Platforms, Inc."}, {"D0:B3:C2", "Meta Platforms, Inc."}, {"D4:D6:59", "Meta Platforms, Inc."}, {"00:03:C5", "Mobotix AG"}, 
                       {"08:EA:40", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, {"0C:8C:24", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, {"0C:CF:89", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, {"10:A4:BE", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, 
                       {"14:5D:34", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, {"14:6B:9C", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, {"20:32:33", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, {"2C:C3:E6", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, 
                       {"30:7B:C9", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, {"34:7D:E4", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, {"38:01:46", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, {"38:7A:CC", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, 
                       {"44:01:BB", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, {"54:EF:33", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, {"60:FB:00", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, {"6C:D5:52", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, 
                       {"74:EE:2A", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, {"78:22:88", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, {"7C:A7:B0", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, {"84:FC:14", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, 
                       {"88:49:2D", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, {"94:BA:06", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, {"98:03:CF", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, {"A0:9F:10", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, 
                       {"A8:B5:8E", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, {"B4:6D:C2", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, {"C4:3C:B0", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, {"C8:FE:0F", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, 
                       {"CC:64:1A", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, {"E0:B9:4D", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, {"EC:3D:FD", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, {"F0:C8:14", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, 
                       {"FC:23:CD", "SHENZHEN BILIAN ELECTRONIC CO.，LTD"}, {"20:F4:1B", "Shenzhen Bilian electronic CO.,LTD"}, {"28:F3:66", "Shenzhen Bilian electronic CO.,LTD"}, {"3C:33:00", "Shenzhen Bilian electronic CO.,LTD"}, 
                       {"44:33:4C", "Shenzhen Bilian electronic CO.,LTD"}, {"AC:A2:13", "Shenzhen Bilian electronic CO.,LTD"}, {"00:1C:27", "Sunell Electronics Co."}, {"00:33:7A", "Tuya Smart Inc."}, 
                       {"10:5A:17", "Tuya Smart Inc."}, {"10:D5:61", "Tuya Smart Inc."}, {"18:69:D8", "Tuya Smart Inc."}, {"18:DE:50", "Tuya Smart Inc."}, 
                       {"1C:90:FF", "Tuya Smart Inc."}, {"20:F1:B2", "Tuya Smart Inc."}, {"30:48:7D", "Tuya Smart Inc."}, {"38:1F:8D", "Tuya Smart Inc."}, 
                       {"38:2C:E5", "Tuya Smart Inc."}, {"38:A5:C9", "Tuya Smart Inc."}, {"3C:0B:59", "Tuya Smart Inc."}, {"4C:A9:19", "Tuya Smart Inc."}, 
                       {"50:8A:06", "Tuya Smart Inc."}, {"50:8B:B9", "Tuya Smart Inc."}, {"68:57:2D", "Tuya Smart Inc."}, {"70:89:76", "Tuya Smart Inc."}, 
                       {"7C:F6:66", "Tuya Smart Inc."}, {"80:64:7C", "Tuya Smart Inc."}, {"84:E3:42", "Tuya Smart Inc."}, {"A0:92:08", "Tuya Smart Inc."}, 
                       {"A8:80:55", "Tuya Smart Inc."}, {"B8:06:0D", "Tuya Smart Inc."}, {"BC:35:1E", "Tuya Smart Inc."}, {"C0:F8:53", "Tuya Smart Inc."}, 
                       {"C4:82:E1", "Tuya Smart Inc."}, {"CC:8C:BF", "Tuya Smart Inc."}, {"D4:A6:51", "Tuya Smart Inc."}, {"D8:1F:12", "Tuya Smart Inc."}, 
                       {"D8:C8:0C", "Tuya Smart Inc."}, {"D8:D6:68", "Tuya Smart Inc."}, {"D8:FC:92", "Tuya Smart Inc."}, {"E4:AE:E4", "Tuya Smart Inc."}, 
                       {"F8:17:2D", "Tuya Smart Inc."}, {"FC:3C:D7", "Tuya Smart Inc."}, {"FC:67:1F", "Tuya Smart Inc."} };   // WiFi MAC match criteria



#endif // DEFAULT_TARGETS_H_