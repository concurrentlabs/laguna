# Laguna #

Laguna is a transparent caching control plane providing IP/MPLS traffic monitoring & analysis, cache definition management & traffic matching.

**Solution Overview**

Service providers and enterprises are facing a large and growing amount of over-the-top (OTT) traffic traversing their IP networks. During peak viewing periods nearly two-thirds of all Internet traffic is related to OTT streaming video services like those operated by Netflix, Google, Amazon, and Apple. Left unmanaged, this traffic can lead to network congestion, lower performing Internet services for consumers, and higher peering costs for service providers.

The transparent caching solution helps service providers better manage network utilization by transparently caching popular Internet content within the service provider’s network. Once content is cached, consumer requests can be fulfilled using local server resources rather than accessing data through the Internet peering point. For the service provider, this reduces congestion on the IP network, lowers peering costs, and provides improved control over network utilization during peak usage periods. From a consumer perspective, transparent caching improves the quality and performance of OTT streaming services.


Laguna is part of an open source transparent caching solution, consisting of the following open source projects:

Laguna

- An open source transparent caching control plane providing traffic monitoring and analysis, policy management and profile matching, and a content caching decision engine
- Sponsored by Concurrent, released under the Apache License Version 2.0
- Available on GitHub at https://github.com/concurrentlabs/laguna


Traffic Control

- An open source implementation of a content delivery network providing HTTP request routing, performance monitoring, and a web-based management console
- Sponsored by Comcast, released under the Apache License Version 2.0
- Available on GitHub at https://github.com/Comcast/traffic_control

Traffic Server

- An open source implementation of an edge caching server providing edge caching, request fulfillment, and content streaming
- Sponsored by the Apache Foundation, released under the Apache License Version 2.0
- Available on GitHub at https://github.com/apache/trafficserver



**Key Features**



*Intelligent Intercept*

> The Laguna solution passively monitors Internet traffic and intercepts requests for cacheable content based on configurable algorithms. This allows the operator to decide which content and websites to cache and how to respond in the event of a consumer request. Caching algorithms are configurable as well, enabling the operator to choose the optimal caching model for their particular situation.

*Flexible Caching Profiles*
> 
> The Laguna solution is designed to cache content from the most popular video websites as well as common sources of software updates. Supported sites include Netflix, Hulu, Amazon Prime, You Tube, Twitch, and others. Software updates for Apple iOS, Microsoft Windows, Android, and others can also be cached. Additional profiles can be added as necessary to optimize cache efficiency.

*Distributed Caches / Intelligent Routing*

> The solution is modular, with separate software modules for traffic monitoring and caching. Caches can be distributed deep into the network in order to deliver optimal efficiency. Traffic monitoring can be managed centrally and traffic can be intelligently routed to the most appropriate cache based on a range of factors.

*Flexible Network Integration*

> Laguna supports out-of-band caching by passively monitoring Internet traffic through an optical tap or mirrored switch port. 


*Total Transparency*

> The Laguna solution operates transparently to preserve the application logic of the client and source website. Authentication, statistics, and advertising are all preserved and no changes are needed for websites or clients to work with the OTT content caching features.


*Cost Savings*

> Serving OTT content from within the service provider network reduces operator’s Internet peering costs by lowering the amount of bandwidth required to support consumer driven traffic. In addition, the use of distributed caches allows the operator to cache content deeper in the network, saving more bandwidth over regional and metro networks.


*Traffic Management*

> In addition to lowering costs, OTT content caching enables operators to soften the network impact of periodic spikes in usage. Without a traffic management solution, bandwidth across service provider’s IP network is unmanaged. When a company or website makes new content or software download available, the operator’s network may be hit by large demand peaks as a result of many simultaneous client devices requesting the same content simultaneously. The Laguna content caching solution reduces the impact of the usage spike by serving repeated requests for the same content from caching servers positioned at the edge of the network.

*Improved Consumer Quality of Experience*

> Delivering content from servers located closer to the subscriber enables a better quality of service for OTT streams. With reduced traffic through the Internet peering point and over the IP backbone, the performance of Internet services is significantly improved for all customers.





**FUNCTIONAL OVERVIEW**

The Transparent Caching Solution is targeted with transparently caching Internet video content that is hosted on web sites external to the operator’s network. There need be no business agreement between the operator and the web site in order to cache the content on the caches within the operator’s network. The caching happens transparently, without requiring changes on the web site or subscriber’s equipment.

The Transparent Caching Solution is composed of the following components:

**1) Laguna**

- Integrates with the data network via optical tap or mirrored switch port provided by the operator. Supports IP networks and MPLS networks.
- Monitors and analyses data traffic
- Intercepts traffic based upon configurable algorithms and cache definition policies

**2) Traffic Control: Consists of three main components**

**a. Traffic Operations**

- Provides web-based UI and configuration capabilities for the system
- Centralized operational platform, providing graphs and dashboards of system activity

**b. Traffic Monitor**

- Monitors edge caches and overall health of the system

**c. Traffic Router**

- Receives client requests for content
- Redirects to an available edge cache in the client’s area

**3) Traffic Server Edge Cache**

- Apache Traffic Server (ATS) instance
- Services client HTTP requests
- Caches internet-sourced content


