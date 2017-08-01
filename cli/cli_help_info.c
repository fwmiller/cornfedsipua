static char *help = "\
<?xml version=\"1.0\"?>\
<help>\
  <command>about</command>\
    <brief>Cornfed SIP User Agent version information</brief>\
    <syntax>about</syntax>\
    <description>Display Cornfed SIP User Agent version information</description>\
\
  <command>answer</command>\
    <brief>Answer an incoming call</brief>\
    <syntax>answer</syntax>\
    <description>Answer an incoming call</description>\
\
  <command>debug</command>\
    <brief>Debug settings</brief>\
    <syntax>debug [ (on | off) ]</syntax>\
    <description>Display or set debug settings</description>\
\
  <command>dial</command>\
    <brief>Initiate an outbound call</brief>\
    <syntax>dial</syntax>\
    <description>Initiate an outbound call</description>\
\
  <command>dialog</command>\
    <brief>Dialog information</brief>\
    <syntax>dialog</syntax>\
    <description>Display dialog information</description>\
\
  <command>dnd</command>\
    <brief>Do not disturb</brief>\
    <syntax>dnd [ (on | off) ]</syntax>\
    <description>Set or clear do not disturb</description>\
\
  <command>dns</command>\
    <brief>DNS settings</brief>\
    <syntax>dns</syntax>\
    <description>Display DNS settings</description>\
\
  <command>hangup</command>\
    <brief>Terminate a connected call</brief>\
    <syntax>hangup</syntax>\
    <description>Terminate a connected call</description>\
\
  <command>help</command>\
    <brief>Help information</brief>\
    <syntax>help [ (syntax | COMMAND) ]</syntax>\
    <description>Help information</description>\
\
  <command>history</command>\
    <brief>Call history</brief>\
    <syntax>history</syntax>\
    <description>Display call history</description>\
\
  <command>local</command>\
    <brief>Local SIP endpoint settings</brief>\
    <syntax>local</syntax>\
    <description>Display local SIP endpoint settings.  Two endpoints are displayed.  The first is the local endpoint, or the IP address and UDP port of the local machine.  The second is the visible endpoint, or the IP address and UDP port that other computers on the Internet see when communicating with your machine.</description>\
\
  <command>log</command>\
    <brief>Log settings</brief>\
    <syntax>log [ (info | event | connection | warning | error) ]</syntax>\
    <description>Manage log settings</description>\
\
  <command>nat</command>\
    <brief>NAT settings</brief>\
    <syntax>nat [ (on | off | stun ADDRESS) ]</syntax>\
    <description>Manage Network Address Translation (NAT) settings</description>\
\
  <command>outbound</command>\
    <brief>Outbound proxy settings</brief>\
    <syntax>outbound [ (host ADDRESS | port NUMBER) ]</syntax>\
    <description>Manage outbound proxy settings</description>\
\
  <command>play</command>\
    <brief>Play .wav file</brief>\
    <syntax>play FILENAME</syntax>\
    <description>Play a .wav file</description>\
\
  <command>record</command>\
    <brief>Record .wav file</brief>\
    <syntax>record [ (start | stop | file FILENAME) ]</syntax>\
    <description>Record the incoming voice to a .wav file</description>\
\
  <command>register</command>\
    <brief>Register with a SIP server</brief>\
    <syntax>(register | reg) [ (user NAME | host ADDRESS | port NUMBER | passwd PASSWORD | send | remove | interval SECONDS | buffer SECONDS | rereg | norereg) ]</syntax>\
    <description>Manage registrations with a SIP server</description>\
\
  <command>refuse</command>\
    <brief>Refuse an incoming call</brief>\
    <syntax>refuse</syntax>\
    <description>Refuse an incoming call</description>\
\
  <command>remote</command>\
    <brief>Remote SIP endpoint settings</brief>\
    <syntax>remote [ (user NAME | host ADDRESS | port NUMBER) ]</syntax>\
    <description>Display remote SIP endpoint settings</description>\
\
  <command>reset</command>\
    <brief>Reset client</brief>\
    <syntax>reset</syntax>\
    <description>Reset client operations</description>\
\
  <command>ringtone</command>\
    <brief>Ringtone settings</brief>\
    <syntax>ringtone [ (file FILENAME | device DEVICENAME) ]</syntax>\
    <description>Manage ringtone settings</description>\
\
  <command>rtp</command>\
    <brief>RTP settings</brief>\
    <syntax>rtp [ stats ]</syntax>\
    <description>Display RTP settings</description>\
\
  <command>soundcard</command>\
    <brief>Soundcard settings</brief>\
    <syntax>soundcard [ (device DEVICENAME | flush) ]</syntax>\
    <description>Manage soundcard settings</description>\
\
  <command>wav</command>\
    <brief>List of queued .wav files to be played</brief>\
    <syntax>wav [ flush ]</syntax>\
    <description>Display a list of queued .wav files to be played</description>\
\
  <command>0123456789ABCD#*</command>\
    <brief>Send DTMF digit during a connected call</brief>\
    <syntax>(0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B | C | D | # | *)+</syntax>\
</help>";
