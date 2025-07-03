#pragma once
#include <string>

static const std::string XmlDirectory = 
R"(
<document type="freeswitch/xml">
    <section name="directory">
        <domain name="{{Account.Domain}}">
            <params>
                <param name="dial-string" value="{presence_id=${dialed_user}@${dialed_domain}}${sofia_contact(${dialed_user}@${dialed_domain})}"/>
            </params>
            <user id="{{Account.ID}}" cacheable="60000">
                <params>
                    <param name="password" value="{{Account.Password}}"/>
                </params>
                <variables>
                    <variable name="toll_allow" value="domestic,international,local"/>
                    <variable name="user_context" value="default"/>
                    <variable name="callgroup" value="default"/>
                    <variable name="effective_caller_id_name" value="{{Account.ID}}"/>
                    <variable name="effective_caller_id_number" value="{{Account.ID}}"/>
                    <variable name="outbound_caller_id_name" value="{{Account.ID}}"/>
                    <variable name="outbound_caller_id_number" value="{{Account.ID}}"/>
                </variables>
            </user>
        </domain>
    </section>
</document>
)";
static const std::string XmlSofia = 
R"(
<document type="freeswitch/xml">
    <section name="configuration" description="Various Configuration">
        <configuration name="sofia.conf" description="sofia Endpoint">
            <profiles>
                <profile name="public">
                    <gateways>
                        {% for gw in Gateways %}
                        <gateway name="{{gw.ID}}">
                            <param name="context" value="{{gw.Context}}"/>
                            <param name="register" value="{{gw.Register}}"/>
                            {% if gw.Register %}
                            <param name="username" value="{{gw.Username}}"/>
                            <param name="auth-username" value="{{gw.AuthUsername}}"/>
                            <param name="password" value="{{gw.Password}}"/>
                            <param name="register-transport" value="{{gw.RegisterTransport}}"/>
                            <param name="realm" value="{{gw.Realm}}"/>
                            <param name="register-proxy" value="{{gw.RegisterProxy}}"/>
                            {% endif %}
                            <param name="proxy" value="{{gw.Proxy}}"/>
                            <param name="outbound-proxy" value="{{gw.OutboundProxy}}"/>
                            <param name="expire-seconds" value="{{gw.ExpireSeconds}}"/>
                            <param name="retry-seconds" value="{{gw.RetrySeconds}}"/>
                            <param name="ping" value="{{gw.Ping}}"/>
                            <param name="caller-id-in-from" value="{{gw.CallerIDInFrom}}"/>
                            <param name="contact-in-ping" value="{{gw.ContactInPing}}"/>
                            {% if gw.FromUser != "" %}
                            <param name="from-user" value="{{gw.FromUser}}"/>
                            {% endif %}
                            {% if gw.FromDomain != "" %}
                            <param name="from-domain" value="{{gw.FromDomain}}"/>
                            {% endif %}
                            {% if gw.ContactParams != "" %}
                            <param name="contact-params" value="{{gw.ContactParams}}"/>
                            {% endif %}
                            <param name="extension-in-contact" value="{{gw.ExtensionInContact}}"/>
                            {% if gw.Extension != "" %}
                            <param name="extension" value="{{gw.Extension}}"/>
                            {% endif %}
                            <variables>
                                {% for var in gw.Variables %}
                                <variable name="{{var.Name}}" value="{{var.Value}}" direction="{{var.Direction}}"/>
                                {% endfor %}
                            </variables>
                        </gateway>
                        {% endfor %}
                    </gateways>
                </profile>
            </profiles>
        </configuration>
    </section>
</document>
)";

static const std::string XmlExtensionGatewayDialplan = 
R"(
<document type="freeswitch/xml">
    <section name="dialplan" description="RE Dial Plan For FreeSwitch">
        <context name="{{InDTO.CallerContext}}">
            <extension name="extension_gateway_dialplan" continue="false">
                <condition field="destination_number" expression="^${destination_number}$" break="on-false">
                    <action application="log" data="用户网关:[{{ReverseAccount}}], 账号主叫[${caller_id_number}} => {{XNumber}}], 被叫[${destination_number}]"/>
                    <action application="set" data="call_timeout=$${call_timeout}"/>
                    <action application="set" data="execute_on_answer_sched=sched_hangup +$${hugup_duration}"/>
                    <action application="set" data="hangup_after_bridge=true"/>
                    <action application="set" data="ringback=$${cn-ring}"/>
                    <action application="set" data="effective_caller_id_name={{XNumber}}"/>
                    <action application="set" data="effective_caller_id_number={{XNumber}}"/>
                    <action application="export" data="nolocal:absolute_codec_string=${global_codec_prefs}"/>
                </condition>
                {% if HangupCause != "" %}
                <condition field="destination_number" expression="^${destination_number}$" break="on-true">
                    <action application="export" data="hangup_cause__={{HangupCause}}"/>
                    <action application="hangup" data="NORMAL_TEMPORARY_FAILURE"/>
                </condition>
                {% endif %}
            </extension>
        </context>
    </section>
</document>
)";
/*

                {{% if CallTimeRange != "" %}}
                <condition wday="1,2,3,4,5,6,7" time-of-day="{{CallTimeRange}}" break="on-false">
                    <anti-action application="export" data="hangup_cause__=时间限制"/>
                    <anti-action application="hangup" data="CALL_REJECTED"/>
                </condition>
                {{% endif %}}
                <condition field="${sofia_contact(default/{{ReverseAccount}})}" expression="^error/user_not_registered$" break="on-true">
                    <action application="hangup" data="GATEWAY_DOWN"/>
                </condition>
                {{% if Recording %}}
                <condition field="${recording_path__}" expression="^$" break="never">
                    <action application="export" data="recording_path__=archive/${strftime(%Y%m%d)}/${uuid}.mp3"/>
                    <action application="set" data="media_bug_answer_req=true"/>
                    <action application="set" data="recording_follow_transfer=true"/>
                    <action application="set" data="enable_file_write_buffering=true"/>
                    <action application="set" data="RECORD_STEREO=true"/>
                    <action application="set" data="RECORD_USE_THREAD=true"/>
                    <action application="set" data="RECORD_APPEND=true"/>
                    <action application="set" data="execute_on_answer_rcd=record_session $${recordings_dir}/${recording_path__}"/>
                </condition>
                {{% endif %}}
                <condition field="destination_number" expression="^${destination_number}$" break="never">
                    <action application="set" data="continue_on_fail=USER_BUSY,NO_USER_RESPONSE"/>
                    <action application="set" data="call_url=${regex(${sofia_contact(default/{{ReverseAccount}})}|^(.+)sip:(.+)@(.+)|%1sip:${destination_number}@%3)}"/>
                    <action application="bridge" data="${call_url}"/>
                    <action application="sleep" data="2000"/>
                    <action application="bridge" data="${call_url}"/>
                </condition>

 */
