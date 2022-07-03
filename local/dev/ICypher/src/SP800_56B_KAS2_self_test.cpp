// ================================================================================================================================
//
#include <cassert>
#include "AutoHeapBuffer.h"
#include "Platform.h"
#include "lumi_random.h"
#include "SP800_56B_KAS2.h"

// ================================================================================================================================
// Test keys and cert chains.
static const char * HOST_RCA_PEM =
"-----BEGIN CERTIFICATE-----\r\n"
"MIIDITCCAgmgAwIBAgIBATANBgkqhkiG9w0BAQsFADAyMTAwLgYDVQQDEydIb3N0\r\n"
"LVJvb3QtQ0EgTz1ISUQgR2xvYmFsIE9VPUJpb21ldHJpY3MwHhcNMTYwNjE2MTkz\r\n"
"NjEyWhcNMTcwNjE2MTkzNjEyWjAyMTAwLgYDVQQDEydIb3N0LVJvb3QtQ0EgTz1I\r\n"
"SUQgR2xvYmFsIE9VPUJpb21ldHJpY3MwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAw\r\n"
"ggEKAoIBAQCqBFBolA5K0dJzac5KjdI8iOBEO1qAlEPgN8zNQhzaCCxtiMmQTDVD\r\n"
"dkiobZezliJC3FL7JGRFkDfKXa0fi7ciG2Ey1uvCiuvIDRdajDJOALhmbdkTo5L2\r\n"
"qSCKL+2mOfm07dE8jkyg8A6MzQbpiNtewzj93o3Rx+3u8hW864X3Rr5oRVsVLJVd\r\n"
"PbJg/GyWNqfizcbaMon5pqEAuhewlzaRk9nGOvN+8nqb3X9fGv+IFOZ06Rb3ufUE\r\n"
"aUodVC6aFLZuDLwfmXK/dJmovBPmM7tI0gzgBu2vqOvnMaO5QoW0Lqv3p3lY9zaj\r\n"
"seAp6hPhHdcAEVQw0bc4Mf4U0QKlKcATAgMBAAGjQjBAMB0GA1UdDgQWBBRINB9h\r\n"
"pUQ0syos6KdWhrdbU7diVDAPBgNVHRMECDAGAQEBAgECMA4GA1UdDwEBAQQEAwIB\r\n"
"BDANBgkqhkiG9w0BAQsFAAOCAQEAOA4UefbrjFajh+6sHPDU1ZhOkVL4IAXtkSQm\r\n"
"WRbRD+wsCa2doAgfGX/Rz1maXTV651F2th0/e4oAhdV/gJOeuLM4/3Rbn8TkZHF3\r\n"
"sq0F6Y3bPZWoyViOTsu/DUU+irqdiybNmgH/gZwigjC38YsILGmfmWVUjsR/8YXS\r\n"
"7xeCFedXhfYBFuALVpQbmHxa6COWpcvrqiTsBD9FL4ip6zE3DrFhWvQSSzUs8FCK\r\n"
"e8psVITGX9z+pPJVvQArQH6OPHW/Nb5AnFF6Ug3ygKCPPUgiqj3x97wnRSCYY25S\r\n"
"LIGTTZkCxGJnWkwui81kGrGnDf2S1SV84xYVqG+mcRk8XeJCsw==\r\n"
"-----END CERTIFICATE-----\r\n";
static const char * HOST_CRT_PEM =
"-----BEGIN CERTIFICATE-----\r\n"
"MIIDHTCCAgWgAwIBAgIBATANBgkqhkiG9w0BAQsFADAyMTAwLgYDVQQDEydIb3N0\r\n"
"LVJvb3QtQ0EgTz1ISUQgR2xvYmFsIE9VPUJpb21ldHJpY3MwHhcNMTUwMTAxMDAw\r\n"
"MDAwWhcNMjUwMTAxMDAwMDAwWjA0MTIwMAYDVQQDEylIb3N0LVRlc3QtQ2VydCBP\r\n"
"PUhJRCBHbG9iYWwgT1U9QmlvbWV0cmljczCCASIwDQYJKoZIhvcNAQEBBQADggEP\r\n"
"ADCCAQoCggEBAMfgNLZz7WgxaMYuoWOE371ea1YTtUljjDzrQBHivzutM8p9cFDr\r\n"
"0O7alVk1vFh1mG+salqH3RAYHakOWWiWT/GpeluMhggyninC94LjlUZrCdr4kB6r\r\n"
"IughN/4p3fWyb3OvdAQwJd8tJKGBAxZUPbOdFVSOQcRKJFia7PoUVQ46kEAyHLjU\r\n"
"15gkXs83ynPqU7ECHu95+6TkNNBkzF/m8clQzfNT3Hrk7taN+CzUo7TzRvI/uAym\r\n"
"xV2Cem8AbJS71/ntgydegQAc+DP5wkI6nyqS3GqUIfxxiNubvIRWaszE55/qVRet\r\n"
"4Ib1/NOpKI59WPw/wFBnoXwVD5jxaV72FgECAwEAAaM8MDowHQYDVR0OBBYEFPgL\r\n"
"dQfQneqOFuzHd+uCykhs2EpKMAkGA1UdEwQCMAAwDgYDVR0PAQEBBAQDAgEgMA0G\r\n"
"CSqGSIb3DQEBCwUAA4IBAQCewlHL/mD4dSAdSw+JJaaGnjK1ebsU+TK64U2/X34c\r\n"
"NBGcxMpLwmYsh3pPT6fnPCMG7LfF+CcTbxKgWluZIKqfyYK2ke93/FSSbkcHqgQ9\r\n"
"frtIHU3TjpdLfBolUxNnNWdp8+1Vorvh/zQlU9FN2tHMj3o2E1xQrrpy2eS2UIw+\r\n"
"goJl+QlOmus18UEVmUqz/zf/M1JJO2EFZUDeC29fDuq6q0gyJ1NhWON0z9l5V7KH\r\n"
"LL5+Y3BD86BYV5NJqB2x9U8VCtBgsCiQVdeEH0kUlgLe0OqfuMi8VXYGU3NR/AX/\r\n"
"AVklk/3LaFvsoyEqas+h+IpeKwOyP6mpvSD8Jy7xneLs\r\n"
"-----END CERTIFICATE-----\r\n";
static const char * HOST_PRV_PEM =
"-----BEGIN RSA PRIVATE KEY-----\r\n"
"MIIEpAIBAAKCAQEAx+A0tnPtaDFoxi6hY4TfvV5rVhO1SWOMPOtAEeK/O60zyn1w\r\n"
"UOvQ7tqVWTW8WHWYb6xqWofdEBgdqQ5ZaJZP8al6W4yGCDKeKcL3guOVRmsJ2viQ\r\n"
"Hqsi6CE3/ind9bJvc690BDAl3y0koYEDFlQ9s50VVI5BxEokWJrs+hRVDjqQQDIc\r\n"
"uNTXmCRezzfKc+pTsQIe73n7pOQ00GTMX+bxyVDN81PceuTu1o34LNSjtPNG8j+4\r\n"
"DKbFXYJ6bwBslLvX+e2DJ16BABz4M/nCQjqfKpLcapQh/HGI25u8hFZqzMTnn+pV\r\n"
"F63ghvX806kojn1Y/D/AUGehfBUPmPFpXvYWAQIDAQABAoIBACCS1PRhGzEuUDVw\r\n"
"FwQpfO8XKqX6C0LdEtdAvY3Mpr66gOTAoaPGcqPigpFp1PqTm5ajgiej2a6MR5Us\r\n"
"Df0h9gBbzuGJmrROT4Fd0UmG/mZd/lqtxIsC9+rdswrcVtx6SniPgPhHwnxHFp+t\r\n"
"b0vdybpyYK4JiwVK0faQtQsiciQhtGvFmAN64ACpzxn9E+In3Q5wW68kVDkELqap\r\n"
"7RmyTTOXceZdizvYctA18NG57CocGS0awTfGzIrKSovX+gI7c28hLFykuYFk0q79\r\n"
"dKNpEQo5gpFLWfCIlKjTJRnttqZ8HvJBB3rk0pv93AgE9lf0kHjOjNZqEn0OJajd\r\n"
"TPCJDuMCgYEA6HJYxU9n2Tr1cJO6OS416EGUwW82vg2mq3rJ0FwR+ABQJYxPldr3\r\n"
"e1F/hbrKhK+82U78+cpPEUORXBpHKcMXs4g1Pv9HQs0//sTYhcceny0YMc57GVXp\r\n"
"KPBSl5nf9FcGLJWaN9dYUfEAkH3fadSvKLhEZ5GzJpiZhO7qAle6aV8CgYEA3CD5\r\n"
"C6TxeGB4uXtNrj6/c8f7f3oaMnLuA/ytyy/2CANmOe8pV2fV/m2DS3fPbIXoS3QF\r\n"
"Bwo+MWWOSV0pAfQ6svkzliWMSqXPawb7R0UmgvMHC53ch9gl25nYEpcB5mbp7Jxq\r\n"
"gqpmwS03HASnR/AV0kZIRXyc5Z5/LgYnhob/3J8CgYEA5afECP2RDtDvmhB6lvAP\r\n"
"Fq9kt1CX92IpX5brOc8debdleBOOtfInPVGh+pA4cqvmm5Us3+aABACL50GXOa2l\r\n"
"Bx317Y3t7BQ5vA7kFhhLKt004FCu0rNMr8Pw9hNzb0djtD0Cp1U9z4ebFScnyrn1\r\n"
"bPfRrboIFIQEeZBK55GhfWUCgYBFRdsZ3v5ec32KYVNA0l5NgVGT/EjiN2jPuGS4\r\n"
"3I8AVpGFCc5owzSErNH0ne+Yp0tC82/hl0ebI+pzC+4rvz/5spKZXZeYpjVv6PKK\r\n"
"qyhDbVd6QGN2HH9NbEyjDQJ8D0buPwsFs7pCcIg3OP+rk9JKKfP0ahK17/r18Nnf\r\n"
"a/x44QKBgQCE4nCLO/HpAH+3MVKErLcb2PeQg1uBV6P7jYiw0GsnIalkI5k64DKg\r\n"
"QSNDd5JhXO/poQUjvi3zd5Mky6RiMWLL8/ar6/HpISP6wFZr0fxpCun4fwIR1vae\r\n"
"ZtY3chSpmWTcRVqHNXlK1uKcx0+vWDCBo/JHPKylFYQ/KWf0PniQQA==\r\n"
"-----END RSA PRIVATE KEY-----\r\n";
static const char * DEV_RCA_PEM =
"-----BEGIN CERTIFICATE-----\r\n"
"MIIDJTCCAg2gAwIBAgIBATANBgkqhkiG9w0BAQsFADA0MTIwMAYDVQQDEylEZXZp\r\n"
"Y2UtUm9vdC1DQSBPPUhJRCBHbG9iYWwgT1U9QmlvbWV0cmljczAeFw0xNTAxMDEw\r\n"
"MDAwMDBaFw0yNTAxMDEwMDAwMDBaMDQxMjAwBgNVBAMTKURldmljZS1Sb290LUNB\r\n"
"IE89SElEIEdsb2JhbCBPVT1CaW9tZXRyaWNzMIIBIjANBgkqhkiG9w0BAQEFAAOC\r\n"
"AQ8AMIIBCgKCAQEApLZLjxCiRY6NDB/Zv5mbnhrGY3rEt3XalqojRSaa6PDOnoQZ\r\n"
"9sF1+z5qmC0IeNFY51C9hYMjDf5E7ueNn4Zn5L1fx3QiEESXVawJX8LZPZxy9bVt\r\n"
"aRdbpOAX5jrm/w1/kYdK7VbYO0O3miV6F9RUxQd2hF6mDgVfFqwgs71QLfI7A0kP\r\n"
"/d+62E34Wi8v6bL0TeF9b42DWx2UZVmW89cgih6ZT2DmQAEsT4SOQoSbod8xbewe\r\n"
"IzOkrtOsnhB/34i8jDp1FYpQJyoSj2NgjSktVoF+aKlsmAJbhF1TwAU8vfotG/lF\r\n"
"ui2aqaoNjBfBOKfsACxx+HfBp5UGIZn+FlM8rQIDAQABo0IwQDAdBgNVHQ4EFgQU\r\n"
"7jf7+LxNtxXB8FWq6ZG60iaTBr4wDwYDVR0TBAgwBgEBAQIBAjAOBgNVHQ8BAQEE\r\n"
"BAMCAQQwDQYJKoZIhvcNAQELBQADggEBABzh332oM4SeTGUplJFKcJv/BoubMZUW\r\n"
"QqIHW57pekMZbB8HWuT7tGBYps14AZwp61S3vpiFvBDV6Ue+An0Wj5BiS4WPnOAS\r\n"
"igfRh0UGduEbK8HjZJZL1/k2bIGH3o9jX5I42eqERv6QZZDL5VtaExpS2bGeK90u\r\n"
"4JtEtoMF7W4uR8IL3EkGlVzWgn9191bJBAPuFi57EE/GhIf65ySsihYysBqh++pq\r\n"
"sa+fi173/xKFG7egctQXEbuGJzsQvLU0vtLjphNkv3VuUv4awz8NyuIjH+aDYA5v\r\n"
"Bp53UuxdEzP9RdkHJZehd1P4Ce4lot+mANbWVFBBeQV2RY+vORG2Rdk=\r\n"
"-----END CERTIFICATE-----\r\n";
static const char * DEV_CHN_PEM =
"-----BEGIN CERTIFICATE-----\r\n"
"MIIDMDCCAhigAwIBAgICDkIwDQYJKoZIhvcNAQELBQAwNDEyMDAGA1UEAxMpRGV2\r\n"
"aWNlLVJvb3QtQ0EgTz1ISUQgR2xvYmFsIE9VPUJpb21ldHJpY3MwHhcNMTUwMTAx\r\n"
"MDAwMDAwWhcNMjUwMTAxMDAwMDAwWjA+MTwwOgYDVQQDEzNEZXZpY2UtSW50ZXJt\r\n"
"ZWRpYXRlLUNBLTAgTz1ISUQgR2xvYmFsIE9VPUJpb21ldHJpY3MwggEiMA0GCSqG\r\n"
"SIb3DQEBAQUAA4IBDwAwggEKAoIBAQDLCSLsSnokGL6O0giET5Ds3fdsl2PDWJLm\r\n"
"FP4O5+qWuT6Js+Nz4NV78JN3YSLLhAMDEHTEAEoDR6iTCgfRFZgnSgN34iUl1WK7\r\n"
"I5XLAhPCL4YNIkcJBTKi2cJHW13PrNVPxwjto/KGBH+lJEabR2RAbSqpLvPV7gbW\r\n"
"l20EgQrNtV21nPiGYOoS6X9vYS6ViF7Y0aQO45jf5E1s7M0s1FaeIYFPqWzTxl6Q\r\n"
"6CfZHFZBN94k/nhHMT8c0SAn2vJzZNz+5026FVlwgMjVUPmFDhUhRkf0uLktXutL\r\n"
"SmyPjFlSdXo6DvNEjUVxheXreTosoCA1/U6OR21MBhQCVnuLQ6BZAgMBAAGjQjBA\r\n"
"MB0GA1UdDgQWBBSkV0ggcDO+xGF82v4qCotS1WVm9TAPBgNVHRMECDAGAQEBAgEB\r\n"
"MA4GA1UdDwEBAQQEAwIBATANBgkqhkiG9w0BAQsFAAOCAQEAPOaPDqQPhpb+ryLb\r\n"
"RUht8p7lkojnwI6N27avK0s2z/kDNcV9EaSmbtvXOIB8sZkSMi0HLczF2KK9Tv0U\r\n"
"x/w+f+imS2+Yc3xzFPfSNljUTaEOy4vZFWsf8v8W7U45p8RxLIh9TAXV/0hAmHQT\r\n"
"jRbxlPlFMxtroJA1DFONwXuI9+qgU4a3ScT8s5nFQHn72+LvtORQKHihk+RNbXPe\r\n"
"5RyyP6UZjCV3KDCxHTrwVescZ+zSPq4JVG/eChMqem9nrlCuTUG5GMyIVT5EG/8X\r\n"
"vleJYlg220aWbDOWMnV+2pe7M6pSEDRt2yhJNek7fzhQHefqtaK4Y1uo1HVnfQNm\r\n"
"wapnlw==\r\n"
"-----END CERTIFICATE-----\r\n"
"-----BEGIN CERTIFICATE-----\r\n"
"MIIDMDCCAhigAwIBAgIBATANBgkqhkiG9w0BAQsFADA+MTwwOgYDVQQDEzNEZXZp\r\n"
"Y2UtSW50ZXJtZWRpYXRlLUNBLTAgTz1ISUQgR2xvYmFsIE9VPUJpb21ldHJpY3Mw\r\n"
"HhcNMTUwMTAxMDAwMDAwWhcNMjUwMTAxMDAwMDAwWjA7MTkwNwYDVQQDEzBWNDMw\r\n"
"LjE4NzI4MTk1NzUtY3J5cHQgTz1ISUQgR2xvYmFsIE9VPUJpb21ldHJpY3MwggEi\r\n"
"MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCzzQepRZJ2eRKNpSk5pTx/Mwsa\r\n"
"6VHwdbzWghC+WK424bmEDieFzQfyH08pJyN1flQE8rJ+AjnMybvWV8I67ZC+SOKA\r\n"
"KJ60RY6hlhnRgdc13bFXIBaGmXlu/mj7TMY9LLAtKJdXZ/wqM8/z+jQSo5gKwXJz\r\n"
"BMHU6ZKFQrC0eUWrSNweYoxE+fvYpf0hJymug98xTw4WCe+hD+fVvXb30cG+b0yO\r\n"
"0dIu0VNkdLU613Ep+iTmxtlt+qBQw5mizJuao9zNq4czNi5RX+oEQAopGMQ9+B7S\r\n"
"5O0MQtrgtsf5M5R1TR5oZXoDcK9ZbhHp+F9NgM5/CdlKxP/qjwNDSZrxtPTtAgMB\r\n"
"AAGjPDA6MB0GA1UdDgQWBBRqr6E+UkhiaWBEYIKOzhEmD+ePdjAJBgNVHRMEAjAA\r\n"
"MA4GA1UdDwEBAQQEAwIBIDANBgkqhkiG9w0BAQsFAAOCAQEALxIrUxHneaQqt6p4\r\n"
"Gbss9IacD0ObSWLDKcaa3YXzTQ/rTKDOEQTp9ZK8apGpsvNRUrd/+4pNWw/1lUQg\r\n"
"F/gDB73fmiRGvuuiicWin/51GqFtWBrJ+HLqd6B4+ix2ZT65Q2oc9P2xu6q8e7EN\r\n"
"EXB5cKCw6HU5Kl86B3rxqBwKWb950zYlb5Z+XtRZp7AuVo74ZxUsRkSfeGn1AwXu\r\n"
"abSzbABdUyx238YH1wRm/vliy8uXHcHyYXlNzLTm+FDlc88lH6AdQYZlAM2wRkbl\r\n"
"U2S10E+PfepOKT7EkxSUrM06hqBbPf3+mkc1Hn766SY5ObrJoc0gRm31aRUZ15eZ\r\n"
"EhoHZw==\r\n"
"-----END CERTIFICATE-----\r\n";
static const char * DEV_PRV_PEM =
"-----BEGIN RSA PRIVATE KEY-----\r\n"
"MIIEogIBAAKCAQEAs80HqUWSdnkSjaUpOaU8fzMLGulR8HW81oIQvliuNuG5hA4n\r\n"
"hc0H8h9PKScjdX5UBPKyfgI5zMm71lfCOu2QvkjigCietEWOoZYZ0YHXNd2xVyAW\r\n"
"hpl5bv5o+0zGPSywLSiXV2f8KjPP8/o0EqOYCsFycwTB1OmShUKwtHlFq0jcHmKM\r\n"
"RPn72KX9IScproPfMU8OFgnvoQ/n1b1299HBvm9MjtHSLtFTZHS1OtdxKfok5sbZ\r\n"
"bfqgUMOZosybmqPczauHMzYuUV/qBEAKKRjEPfge0uTtDELa4LbH+TOUdU0eaGV6\r\n"
"A3CvWW4R6fhfTYDOfwnZSsT/6o8DQ0ma8bT07QIDAQABAoIBABF0JeS9f1tnIwZt\r\n"
"/Hj/PHFVPqQ6TFJ302t398Z1LFWL3EA18Ynls8+XUM4RYbn47TtdEIz2jAQq/G7G\r\n"
"WRvy+MSH+w4ZeK0jksDGezN2aQeZFtXQJG+k0VVqy+fCdadK7GqN0Kx9ncxo+Cxa\r\n"
"Yu4A7x7y1WMrQzehrtqnH9PScTwQWIYRSQEqqi0Ui6+ryslRI6/29/daYxEYOuA1\r\n"
"WMaZiVA5MziNi1lD975IGPvRCFYzXvOtnTeu34t1uchobMBTdmtAaPut2o2xm5Z6\r\n"
"zSRJ3rpsA9Ri6yc3fZ2jsXOVpfVb4Q1c9nnibfgkA7zwhR4n61wu55czhIrpmhMx\r\n"
"NyLpsu8CgYEA/LOhVlxOYntw2TXE/TyyVnA/j+Yw0ywoRJpFyj5cDp4PsI5p3ObK\r\n"
"iCmWNkg+Fl11rHTIlSjLXe0SxkypmkfHwokjsEZI155K8Db8U/R8YG0wc/XmhvZv\r\n"
"4bpvV2Jp/IlwdeeTU6I9TJDHPom4NN2pn0g6/fwQ7rpDxHMrrR0zEicCgYEAtiXP\r\n"
"rIgS+12Kaw/NMYtChWqvk7c8E/CfxS1cckBPvJ6NVLg+waCpl028EsXMQj/1JJpz\r\n"
"rX4WRJGwAs67+kxkhQktRSHqkqe6T31D40+XYR1LDeJnGgGSJ0QZDYiqesD3SYip\r\n"
"Dta3+ct9Phgfn2u6VqNwSB5ng6JJt3zNQTjn8MsCgYBegNr8TgbuY5WfVbF2twVG\r\n"
"uDG9IevufVwN09f3u3x35vqdH5VNjG3/n/XSFRBLEuaX+RCuX5Q2axuTV0YyMVfu\r\n"
"U5UqLg69vC/wR8/zocIF2a++Hh75KB5NP4i2GFLko42QOmtpYookIRyyMZTXCHKP\r\n"
"49ZT97lpXfVL+XpDAmGADwKBgE2JhwAQ5NOwHA47KcQfXe/hXGBrLd3nHnwXNTVF\r\n"
"fyvx6oPX8o8GT2sjIh5zqOHCzB4KyZdfByelROHNE2QXROsE3wqAx5QqZjsUPePz\r\n"
"q6MhjyOX7v+5D3g7mtaSCNA8eIgK7rxsVuC06NmX904r06Sx6kgo+TOlz++V5uIO\r\n"
"0V1FAoGAETFROnER/7R0ZFqCLf0AZCvgsVRkg2t6t4rcfFCoow8O5kpSuDfC9ffA\r\n"
"jLFB5hrSpLX6LTOrYo6ZVMdtzoPYllI7/0gMc5Tp8+qThCVEeeq2hJ98CEuh2l6c\r\n"
"KNQY1AMbJpi/i1RVpWyy12bL43R7sgM39xv2Q6mvK1eZVhpmP7Y=\r\n"
"-----END RSA PRIVATE KEY-----\r\n";
//
// ================================================================================================================================

// ================================================================================================================================
// static seed for lumi_random so we get repeatable results.
static const unsigned char static_seed[48] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F
};

// ================================================================================================================================
// EXPECTED OUTPUT FOR RAW RSA
static const unsigned char xpct_MacTagV[32] =
{
    0x13, 0x6B, 0x1C, 0xE3, 0xED, 0xBD, 0xAF, 0x9E,
    0x2D, 0xA1, 0x8D, 0x25, 0xA7, 0x78, 0xFE, 0x64,
    0xBB, 0xE1, 0xFE, 0x08, 0x1B, 0x04, 0x08, 0xF7,
    0x60, 0x7A, 0x8A, 0x56, 0x9C, 0xC0, 0xCF, 0xE0
};

static const unsigned char xpct_MacTagU[32] =
{
    0x6F, 0x69, 0xAF, 0x0F, 0x55, 0xC2, 0x4C, 0x92,
    0x76, 0xA6, 0xD9, 0x60, 0x6C, 0x26, 0x0B, 0x63,
    0xED, 0xCB, 0x10, 0xE6, 0x45, 0xCC, 0x67, 0x97,
    0xCF, 0xC0, 0x0C, 0x08, 0x81, 0x2F, 0xC1, 0x24
};

static const unsigned char xpct_EH2D[32] =
{
    0xD7, 0x7A, 0xE9, 0x9B, 0x81, 0xE7, 0x3A, 0x53,
    0x1F, 0xA6, 0xF1, 0x71, 0xE7, 0xDF, 0xAF, 0x3C,
    0x69, 0x67, 0x56, 0x3A, 0xD1, 0xF7, 0xFE, 0xE4,
    0xE6, 0x8A, 0xB3, 0xCA, 0x12, 0x52, 0xDF, 0x20
};

static const unsigned char xpct_ED2H[32] =
{
    0x02, 0xE1, 0x38, 0x10, 0x52, 0xC9, 0x43, 0xD2,
    0x33, 0x30, 0x38, 0x7C, 0x46, 0x2E, 0x0A, 0xC3,
    0x90, 0xE1, 0x0B, 0xAF, 0x4C, 0x20, 0x60, 0x16,
    0x93, 0xDA, 0x60, 0x5A, 0x01, 0x1D, 0xE7, 0xBA
};

static const unsigned char xpct_MH2D[32] =
{
    0x3E, 0x62, 0x4A, 0x5B, 0xB3, 0x39, 0xCC, 0xED,
    0x23, 0xDB, 0xFD, 0x02, 0x38, 0xF5, 0x76, 0xDD,
    0xD0, 0x47, 0x9B, 0x48, 0xDB, 0xEF, 0x56, 0xAB,
    0x02, 0x56, 0x42, 0x0A, 0x09, 0x29, 0x1C, 0x36
};

static const unsigned char xpct_MD2H[32] =
{
    0xF2, 0x70, 0x08, 0xBB, 0x36, 0x49, 0x5D, 0x12,
    0x2B, 0x08, 0xDB, 0x5F, 0xFC, 0xAC, 0x67, 0xF6,
    0x42, 0x91, 0x8E, 0x3D, 0x79, 0x27, 0x9E, 0x5B,
    0x0E, 0xAA, 0xA6, 0x75, 0xC9, 0xA2, 0x53, 0xB6
};
//
// ================================================================================================================================

// ================================================================================================================================
// perform the work
static bool SP800_56B_KAS2_self_test_2(SP800_56B_KAS2::RSA_MODE mode)
{
    SP800_56B_KAS2* u_kex = new SP800_56B_KAS2();
    if( u_kex == NULL) { return false; }
    AutoDestructObject<SP800_56B_KAS2> u_k(u_kex);


    SP800_56B_KAS2* v_kex = new SP800_56B_KAS2();
    if(v_kex == NULL) { return false;}
    AutoDestructObject<SP800_56B_KAS2> v_k(v_kex);

    bool rc_u_init = u_kex->U_Init(HOST_CRT_PEM, strlen(HOST_CRT_PEM), HOST_PRV_PEM, strlen(HOST_PRV_PEM),
                                   DEV_CHN_PEM,  strlen(DEV_CHN_PEM),  DEV_RCA_PEM, strlen(DEV_RCA_PEM));
    if (!rc_u_init)
        return false;
    bool rc_v_init = v_kex->V_Init(DEV_CHN_PEM,  strlen(DEV_CHN_PEM),  DEV_PRV_PEM,  strlen(DEV_PRV_PEM),
                                   HOST_CRT_PEM, strlen(HOST_CRT_PEM), HOST_RCA_PEM, strlen(HOST_RCA_PEM));
    if (!rc_v_init)
        return false;

    u_kex->Set_Mode(mode);
    //v_kex->Set_Mode(mode);    -- XXX - jbates - V gets its mode from the U_Phase_1 cryptogram.

    // transport buffer
    AutoHeapBuffer buf(3072);
    AutoHeapBuffer fub(3072);
    size_t len;

    // ============================================================================================================================
    // U INITIATING
    // PHASE 1
    buf.Clear();
    len = buf.Len();
    bool rc_u_phase_1 = u_kex->U_Phase_1(buf.u8Ptr(), len);
    if (!rc_u_phase_1)
        return false;

    // ============================================================================================================================
    // V RECEIVING
    // PHASE 2
    bool rc_v_phase_2 = v_kex->V_Phase_2(buf.u8Ptr(), len);
    if (!rc_v_phase_2)
        return false;

    // ============================================================================================================================
    // V RESPONDING
    // PHASE 3
    buf.Clear();
    len = buf.Len();
    bool rc_v_phase_3 = v_kex->V_Phase_3(buf.u8Ptr(), len);
    if (!rc_v_phase_3)
        return false;

    // ============================================================================================================================
    // ASSERT MacTagV matches - presupposes knowing on-wire format...
    if (len < 32)
        return false;
    //const uint8_t * MacTagV = buf.u8Ptr() + len - 32;
    //if (memcmp(MacTagV, xpct_MacTagV, 32))
    //    return false;

    // ============================================================================================================================
    // U VERIFYING
    // PHASE 4
    bool rc_u_phase_4 = u_kex->U_Phase_4(buf.u8Ptr(), len);
    if (!rc_u_phase_4)
        return false;

    // ============================================================================================================================
    // U RESPONDING
    // PHASE 5
    buf.Clear();
    len = buf.Len();
    bool rc_u_phase_5 = u_kex->U_Phase_5(buf.u8Ptr(), len);
    if (!rc_u_phase_5)
        return false;

    // ============================================================================================================================
    // ASSERT MacTagU matches - presupposes knowing on-wire format...
    if (len < 32)
        return false;
    //const uint8_t * MacTagU = buf.u8Ptr() + len - 32;
    //if (memcmp(MacTagU, xpct_MacTagU, 32))
    //    return false;

    // ============================================================================================================================
    // V VERIFYING
    // PHASE 6
    bool rc_v_phase_6 = v_kex->V_Phase_6(buf.u8Ptr(), len);
    if (!rc_v_phase_6)
        return false;

    // ============================================================================================================================
    // ASSERT COMPLETE
    bool rc_complete;
    rc_complete = v_kex->Assert_Complete();
    if (!rc_complete)
        return false;
    rc_complete = u_kex->Assert_Complete();
    if (!rc_complete)
        return false;

    // ============================================================================================================================
    // ASSERT ALL PRODUCED KEYS MATCH
#if 0
// XXX - jbates - disabled due to API change...
    bool rc_get_key;
    // V's KEYS
    rc_get_key = v_kex->Get_EH2D(buf.u8Ptr(), 32);
    if (!rc_get_key)
        return false;
    rc_get_key = u_kex->Get_EH2D(fub.u8Ptr(), 32);
    if (!rc_get_key)
        return false;
    if (memcmp(buf.u8Ptr(), fub.u8Ptr(), 32))
        return false;

    rc_get_key = v_kex->Get_ED2H(buf.u8Ptr(), 32);
    if (!rc_get_key)
        return false;
    rc_get_key = u_kex->Get_ED2H(fub.u8Ptr(), 32);
    if (!rc_get_key)
        return false;
    if (memcmp(buf.u8Ptr(), fub.u8Ptr(), 32))
        return false;

    rc_get_key = v_kex->Get_MH2D(buf.u8Ptr(), 32);
    if (!rc_get_key)
        return false;
    rc_get_key = u_kex->Get_MH2D(fub.u8Ptr(), 32);
    if (!rc_get_key)
        return false;
    if (memcmp(buf.u8Ptr(), fub.u8Ptr(), 32))
        return false;

    rc_get_key = v_kex->Get_MD2H(buf.u8Ptr(), 32);
    if (!rc_get_key)
        return false;
    rc_get_key = u_kex->Get_MD2H(fub.u8Ptr(), 32);
    if (!rc_get_key)
        return false;
    if (memcmp(buf.u8Ptr(), fub.u8Ptr(), 32))
        return false;

    return true;
#endif
    // XXX - jbates - because we are not performing full test, always return false here...
    return false;

}

#include <stdio.h>

// ================================================================================================================================
// setup statically-seeded lumi_random, perform work, return result.
bool SP800_56B_KAS2_self_test()
{
    bool bRC;
    lumi_random_static_seed_init(static_seed, sizeof(static_seed));
    bRC = SP800_56B_KAS2_self_test_2(SP800_56B_KAS2::MODE_RSA_RAW);
    if (bRC)
    {
        fprintf(stderr, "SP800_56B_KAS2 passes in RAW_RSA mode\n");
        lumi_random_static_seed_init(static_seed, sizeof(static_seed));
        bRC = SP800_56B_KAS2_self_test_2(SP800_56B_KAS2::MODE_RSA_PKCS1_V15);
        if (bRC)
        {
            fprintf(stderr, "SP800_56B_KAS2 passes in PKCS1 v15 mode\n");
            lumi_random_static_seed_init(static_seed, sizeof(static_seed));
            bRC = SP800_56B_KAS2_self_test_2(SP800_56B_KAS2::MODE_RSA_PKCS1_V21);
            if (bRC)
            {
                fprintf(stderr, "SP800_56B_KAS2 passes in PKCS1 v21 mode\n");
            }
        }
    }

    lumi_random_static_seed_init(NULL, 0);
    return bRC;
}
//
// ================================================================================================================================
