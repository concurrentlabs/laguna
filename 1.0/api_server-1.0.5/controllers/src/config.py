#!/usr/bin/env python
#

from threading import Lock
import Queue

"""
Read config and server yaml files.
"""

update_lock = Lock()

# Line below MUST come after update_lock...
from config_functions import read

# Config section...
# IMPORTANT - config.yaml should be ../config.yaml and servers.yaml should be ../servers.yaml on Red Hat!
debug = ""
debug_ip = ""  # NOTE: This flag should ONLY come into play if debug: False in the config.yaml file! It is only for GeoIP QA testing.
port = ""
http_response_codes = {}
logging = ""
log_file = ""
errors_log_file = ""
log_format = ""
log_date_format = ""
log_when = ""
log_utc = ""
log_rotation = ""
log_backups = ""
cookie_expires = {}
alert_email = {}

api_ip_allow_range = []

keys = {}

#Server section...

servers_with_weights = {}
servers = []
servers_weighted = []

server_count = 0
servers_weighted_count = 0

server_defaults = {}
server_default_port = ":80"

server_ports = {}

servers_interface_ip = []

servers_server_config = {}

servers_purge = {}

# Start the threading queues...
server_queue = Queue.Queue()

# End global variable section for config

# Set the above variables via the read function from config_functions.py
read()
