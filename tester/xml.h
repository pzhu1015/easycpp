#pragma once
#include "xml_serialize.h"
#include "datetime.h"
#include "obj.h"

REGIST_CLASS_XML(::datetime::DateTime, "datetime");

REGIST_MEMBER_XML(
    test::Registration,
    "registration",
    NAME(CallId,"call-id"),
    NAME(User,"user"),
    NAME(Contact,"contact"),
    NAME(Agent,"agent"),
    NAME(Status,"status"),
    NAME(PingStatus,"ping-status"),
    NAME(PingTime,"ping-time"),
    NAME(Host,"host"),
    NAME(NetworkIp,"netework-ip"),
    NAME(NetworkPort,"network-port"),
    NAME(SipAuthUser,"sip-auth-user"),
    NAME(SipAuthRealm,"sip-auth-realm"),
    NAME(MwiAccount,"mwi-account")
);

REGIST_MEMBER_XML(
    test::Extension,
    "profile",
    NAME(Registrations, "registrations")
);

REGIST_MEMBER_XML(
    test::Gateway,
    "gateway",
    NAME(Name,"name"),
    NAME(Profile,"profile"),
    NAME(Schema,"schema"),
    NAME(Realm,"realm"),
    NAME(UserName,"username"),
    NAME(Password,"password"),
    NAME(From,"from"),
    NAME(Contact,"contact"),
    NAME(Exten,"exten"),
    NAME(To,"to"),
    NAME(Proxy,"proxy"),
    NAME(Context,"context"),
    NAME(Expire,"expire"),
    NAME(Freq,"freq"),
    NAME(Ping,"ping"),
    NAME(PingFreq,"pingfreq"),
    NAME(PingMin,"pingmin"),
    NAME(PingCount,"pingcount"),
    NAME(PingMax,"pingmax"),
    NAME(PintTime,"pingtime"),
    NAME(Pinging,"pinging"),
    NAME(State,"state"),
    NAME(Status,"status"),
    NAME(UpTimeUsec,"uptime-usec"),
    NAME(CallsIn,"calls-in"),
    NAME(CallsOut,"calls-out"),
    NAME(FailedCallsIn,"failed-calls-in"),
    NAME(FailedCallsOut,"failed-calls-out")
);

