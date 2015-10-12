# laguna
Laguna is a transparent caching control plane providing IP/MPLS traffic monitoring &amp; analysis, cache definition management &amp; traffic matching.
<p>
OVERVIEW
</p>
Laguna is part of an open source transparent caching solution, consisting of the following open source projects:

<b>Laguna</b>
<li>
•	An open source transparent caching control plane providing traffic monitoring and analysis, policy management and profile matching, and a content caching decision engine
</li>
<li>
•	Sponsored by Concurrent, released under the Apache License Version 2.0
</li>
<li>
•	Available on GitHub at https://github.com/concurrentlabs/laguna 
</li>
<br/>
<b>Traffic Control</b>
<li>
•	An open source implementation of a content delivery network providing HTTP request routing, performance monitoring, and a web-based management console
</li>
<li>
•	Sponsored by Comcast, released under the Apache License Version 2.0
</li>
<li>
•	Available on GitHub at https://github.com/Comcast/traffic_control 
</li>
<br/>
<b>Traffic Server</b>
<li>
•	An open source implementation of an edge caching server providing edge caching, request fulfillment, and content streaming
</li>
<li>
•	Sponsored by the Apache Foundation, released under the Apache License Version 2.0
</li>
<li>
•	Available on GitHub at https://github.com/apache/trafficserver 
</li>
<p>
<br/>

<b>FUNCTIONAL OVERVIEW
</b></p>
<p>
The Transparent Caching Solution is targeted with transparently caching Internet video content that is hosted on web sites external to the operator’s network.  There need be no business agreement between the operator and the web site in order to cache the content on the caches within the operator’s network.  The caching happens transparently, without requiring changes on the web site or subscriber’s equipment.
</p>
<p>
The Transparent Caching Solution is composed of the following components:
</p>
<p>
<b>
1)	Laguna
</b>
<li>a.	Integrates with the data network via optical tap or mirrored switch port provided by the operator.  Supports IP networks and MPLS networks.
</li>
<li>
b.	Monitors and analyses data traffic
</li>
<li>
c.	Intercepts traffic based upon configurable algorithms and cache definition policies
</li>
</p>
<p>
<b>
2)	Traffic Control: Consists of three main components
</b>
</p>
<b>
a.	Traffic Operations
</b>
<li>
i.	Provides web-based UI and configuration capabilities for the system
</li>
<li>
ii.	Centralized operational platform, providing graphs and dashboards of system activity
</li>
</p>
<p>
<b>
b.	Traffic Monitor
</b>
<li>
i.	Monitors edge caches and overall health of the system
</li>
<p><b>
c.	Traffic Router
</b></p>
<li>
i.	Receives client requests for content
</li>
<li>
ii.	Redirects to an available edge cache in the client’s area
</li>
</p>
<p>
<b>
3)	Traffic Server Edge Cache
</b>
<li>
a.	Apache Traffic Server (ATS) instance
</li>
<li>
b.	Services client HTTP requests
</li>
<li>
c.	Caches internet-sourced content
</li>
</p>
<p>
<b>
NETWORK INTEGRATION
</b>
</p>
<p>
Laguna integrates with the operator’s network via an optical network tap or a mirrored switch port providing traffic to a network interface monitored by Laguna.  This tap is an interface on a network switch, and the “tapping” can be done anywhere in the network where Internet traffic can be accessed.  
</p>
<p>
The purpose of the tap is to relay this Internet traffic to Laguna. This relay occurs in parallel to the outbound transfer of the traffic to the Internet.  Thus, a client's Internet traffic travels simultaneously both to Laguna as well as to the Internet site requested by the client.
</p>
<p>
Laguna monitors the traffic to determine if the request is for potentially cacheable content, based upon the cache definition profiles previously provisioned on Laguna.  If the request matches the potentially cacheable criteria, Laguna sends an HTTP 302 redirect to the client, directing him to the Traffic Router for the requested content; Laguna also terminates the client-origin connection by sending resets to the origin. Once the Traffic Router receives the redirected client request it will select an available Apache Traffic Server edge cache instance and then redirect the client to the Apache Traffic server edge cache.  The client typically remembers the redirection so subsequent requests for data within the same session go direct to the edge cache.
</p>
<p>
Client authentication request exchanges are not handled by Laguna or the transparent caching system, so once the first data request is seen, the business logic will already have been validated by the internet site. This ensures that only valid requests are cached and provided by the transparent cache system.
</p>
<p>
<b>Currently Available Cache Definition Profiles</b>
</p>
<p>
The following profiles are currently available for the Laguna system.   Note that only HTTP (port 80) traffic is supported, although MPLS traffic can also be filtered, based on MPLS label.
</p>
<p>
Video sites
</p>
-	Netflix
-	Youtube
-	Hulu
-	Dailymotion
-	Amazon Prime
-	Twitch
-	ESPN
-	HBO Go
-	BBC iPlayer
-	Vimeo
-	LiveLeak
-	Ustream
<p>
Software update sites
</p>
-	Apple iOS
-	Windows Desktop OS updates
-	Android updates
<p>
Game sites
</p>
-	Steam
-	Blizzard





