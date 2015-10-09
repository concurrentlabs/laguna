# laguna
Laguna is transparent caching control plane providing IP/MPLS traffic monitoring &amp; analysis, cache definition management &amp; traffic matching.

OVERVIEW
Laguna is part of a larger solution, consisting of the following open source projects:

Laguna
•	An open source transparent caching control plane providing traffic monitoring and analysis, policy management and profile matching, and a content caching decision engine
•	Sponsored by Concurrent, released under the Apache License Version 2.0
•	Available on GitHub at https://github.com/concurrentlabs/laguna 
Traffic Control
•	An open source implementation of a content delivery network providing HTTP request routing, performance monitoring, and a web-based management console
•	Sponsored by Comcast, released under the Apache License Version 2.0
•	Available on GitHub at https://github.com/Comcast/traffic_control 
Traffic Server
•	An open source implementation of an edge caching server providing edge caching, request fulfillment, and content streaming
•	Sponsored by the Apache Foundation, released under the Apache License Version 2.0
•	Available on GitHub at https://github.com/apache/trafficserver 

FUNCTIONAL OVERVIEW
The Transparent Caching Solution is targeted with transparently caching Internet video content that is hosted on web sites external to the operator’s network.  There need be no business agreement between the operator and the web site in order to cache the content on the caches within the operator’s network.  The caching happens transparently, without requiring changes on the web site or subscriber’s equipment.
The Transparent Caching Solution is composed of the following components:
1)	Laguna
a.	Integrates with the data network via optical tap or mirrored switch port provided by the operator.  Supports IP networks and MPLS networks.
b.	Monitors and analyses data traffic
c.	Intercepts traffic based upon configurable algorithms and cache definition policies
2)	Traffic Control: Consists of three main components
a.	Traffic Operations
i.	Provides web-based UI and configuration capabilities for the system
ii.	Centralized operational platform, providing graphs and dashboards of system activity
b.	Traffic Monitor
i.	Monitors edge caches and overall health of the system
c.	Traffic Router
i.	Receives client requests for content
ii.	Redirects to an available edge cache in the client’s area
3)	Traffic Server Edge Cache
a.	Apache Traffic Server (ATS) instance
b.	Services client HTTP requests
c.	Caches internet-sourced content


NETWORK INTEGRATION
Laguna integrates with the operator’s network via an optical network tap or a mirrored switch port providing traffic to a network interface monitored by Laguna.  This tap is an interface on a network switch, and the “tapping” can be done anywhere in the network where Internet traffic can be accessed.  

The purpose of the tap is to relay this Internet traffic to Laguna. This relay occurs in parallel to the outbound transfer of the traffic to the Internet.  Thus, a client's Internet traffic travels simultaneously both to Laguna as well as to the Internet site requested by the client.

Laguna monitors the traffic to determine if the request is for potentially cacheable content, based upon the cache definition profiles previously provisioned on Laguna.  If the request matches the potentially cacheable criteria, Laguna sends an HTTP 302 redirect to the client, directing him to the Traffic Router for the requested content; Laguna also terminates the client-origin connection by sending resets to the origin. Once the Traffic Router receives the redirected client request it will select an available Apache Traffic Server edge cache instance and then redirect the client to the Apache Traffic server edge cache.  The client typically remembers the redirection so subsequent requests for data within the same session go direct to the edge cache.

Client authentication request exchanges are not handled by Laguna or the transparent caching system, so once the first data request is seen, the business logic will already have been validated by the internet site. This ensures that only valid requests are cached and provided by the transparent cache system.




