#!/usr/bin/env python
#

import json
import datetime
import config
import shutil
import collections

def synchronized(lock):
    """Synchronization decorator."""

    def wrap(f):
        def new_function(*args, **kw):
            lock.acquire()
            try:
                return f(*args, **kw)
            finally:
                lock.release()
        return new_function
    return wrap


def value_to_json(key_value_name, key_value, start=True, stop=True):
    if start:
        start_brace = "{"
    else:
        start_brace = " "

    if stop:
        stop_brace = "}"
    else:
        stop_brace = ","

    return "%s\"%s\": %s%s" % (start_brace, key_value_name, json.dumps(key_value), stop_brace)


def list_to_json_array(value_list, array_name, value_name=None, start=True, stop=True):
    """
    Creates a json array from a list. If the optional value_name is passed then the list becomes a json array of dictionary objects.
    """
    if start:
        start_brace = "{"
    else:
        start_brace = " "

    if stop:
        stop_brace = "}"
    else:
        stop_brace = ","
    if value_name is None:
        return "%s\"%s\": %s%s" % (start_brace, array_name, json.dumps(value_list), stop_brace)

    lst = []

    for value in value_list:
        data_dict = {value_name: value}
        lst.append(data_dict)

    return "%s\"%s\": %s%s" % (start_brace, array_name, json.dumps(lst), stop_brace)


def value_to_yaml(key_value_name, key_value, trailing_line_feed=True):

    if isinstance(key_value, str):
        #format_string = "%s: \n    '%s'\n"
        format_string = "%s: '%s'\n"
    else:
        #format_string = "%s: \n    %s\n"
        format_string = "%s: %s\n"

    return format_string % (convert(key_value_name), key_value) + ('\n' if trailing_line_feed else '')


def list_to_yaml(list_name, value_list, trailing_line_feed=True):
    yaml_data = "%s:\n" % convert(list_name)
    for data in value_list:
        if data is None:
            continue
        yaml_data += "   - %s\n" % data

    return yaml_data + ("\n" if trailing_line_feed else '')


def dict_to_yaml(dict_name, value_dict, trailing_line_feed=True):
    yaml_data = "%s:\n" % convert(dict_name)
    for data in value_dict:
        if value_dict[data] is None:
            value_dict[data] = ''
        if isinstance(value_dict[data], str):
            format_string = "    %s: '%s'\n"
        else:
            format_string = "    %s: %s\n"

        yaml_data += format_string % (data, value_dict[data])

    return yaml_data + ("\n" if trailing_line_feed else '')


def json_to_yaml(json_dict, level=0, spaces=0):  # Returns an empty value if nothing there.
    yaml = ''

    sorted_dict = sorted(json_dict)
    space = level * 4

    first_pass = False
    for sort in sorted_dict:
        data = sort

        if first_pass is True:
            yaml += '%s' % (' ' * (space + spaces))
        first_pass = True

        if isinstance(json_dict[data], unicode):
            json_dict[data] = convert(json_dict[data])
        if isinstance(json_dict[data], dict):
            yaml += dict_to_yaml(data, json_dict[data], trailing_line_feed=False)
        elif isinstance(json_dict[data], list):
            #yaml += list_to_yaml(data, json_dict[data], trailing_line_feed=False)
            yaml += '%s:\n' % convert(data)
            for data_list in json_dict[data]:
                yaml += '%s - ' % (' ' * (space + 4))
                yaml += json_to_yaml(data_list, level=level + 1, spaces=3)  # spaces line up the text
        elif isinstance(json_dict[data], str) or \
                isinstance(json_dict[data], int) or \
                isinstance(json_dict[data], long) or \
                isinstance(json_dict[data], float):  # Date/Time should be represented as a string in json - maybe?
            yaml += value_to_yaml(data, json_dict[data], trailing_line_feed=False)

#    for data in json_dict:
#        if first_pass is True:
#            yaml += '%s' % (' ' * space)
#        first_pass = True

#        if isinstance(json_dict[data], unicode):
#            json_dict[data] = convert(json_dict[data])
#        if isinstance(json_dict[data], dict):
#            yaml += dict_to_yaml(data, json_dict[data], trailing_line_feed = False)
#        elif isinstance(json_dict[data], list):
            #yaml += list_to_yaml(data, json_dict[data], trailing_line_feed=False)
#            yaml += '%s:\n' % convert(data)
#            for data_list in json_dict[data]:
#                yaml += '%s - ' % (' ' * (space + 4))
#                yaml += json_to_yaml(data_list, level + 1)
#        elif isinstance(json_dict[data], str) or \
#                isinstance(json_dict[data], int) or \
#                isinstance(json_dict[data], long) or \
#                isinstance(json_dict[data], float):  # Date/Time should be represented as a string in json - maybe?
#            yaml += value_to_yaml(data, json_dict[data], trailing_line_feed = False)

    return yaml


# Used to convert to string objects from unicode objects
def convert(input_value):
    if isinstance(input_value, dict):
        return {convert(key): convert(value) for key, value in input_value.iteritems()}
    elif isinstance(input_value, list):
        return [convert(element) for element in input_value]
    elif isinstance(input_value, unicode):
        return input_value.encode('utf-8')
    else:
        return input_value


# yaml write function
def yaml_write(file_section, file_name):
    try:
        yaml_data = "# DO NOT MODIFY: Machine Generated YAML File!\n# ALL values MUST NOT BE EMPTY! If the value needs to be empty then use a 0 for a numeric value or '' for a string.\n\n"

        # NOTE: Any value that is None will be assumed to be an empty string ''.
        dest_file = file_name + "_" + str(datetime.datetime.now()).replace("-", "").replace(" ", "").replace(":", "").replace(".", "")

        shutil.copyfile(file_name, "yamls/" + dest_file)

        if file_section == "config":
            yaml_data += value_to_yaml("debug", config.debug)
            yaml_data += value_to_yaml("port", config.port)
            yaml_data += dict_to_yaml("http_response_codes", config.http_response_codes)
            yaml_data += value_to_yaml("logging", config.logging)
            yaml_data += value_to_yaml("log_file", config.log_file)
            yaml_data += value_to_yaml("errors_log_file", config.errors_log_file)
            yaml_data += value_to_yaml("log_format", config.log_format)
            yaml_data += value_to_yaml("log_date_format", config.log_date_format)
            yaml_data += value_to_yaml("log_utc", config.log_utc)
            yaml_data += value_to_yaml("log_when", config.log_when)
            yaml_data += value_to_yaml("log_rotation", config.log_rotation)
            yaml_data += value_to_yaml("log_backups", config.log_backups)
            yaml_data += dict_to_yaml("log_message_formats", config.log_message_formats)
            yaml_data += dict_to_yaml("cookie_expires", config.cookie_expires)
            yaml_data += dict_to_yaml("alert_email", config.alert_email)
            yaml_data += dict_to_yaml("geo", config.geo)
            yaml_data += list_to_yaml("geo_country_codes", config.geo_country_codes)
            yaml_data += dict_to_yaml("keys", config.keys)
            yaml_data += value_to_yaml("token_auth", config.token_auth)
            yaml_data += dict_to_yaml("keys_seachange", config.keys_seachange)
            yaml_data += dict_to_yaml("seachange", config.seachange)
            yaml_data += list_to_yaml("api_ip_allow_range", config.api_ip_allow_range)
            yaml_data += dict_to_yaml("ip", config.ip)
            yaml_data += list_to_yaml("ip_allow_range", config.ip_allow_range)
            yaml_data += list_to_yaml("ip_deny_range", config.ip_deny_range)
        elif file_section == "servers":
            if config.routing == 0:
                routing_type = "random"
            elif config.routing == 2:
                routing_type = "snmp"
            else:
                routing_type = "round_robin"
            yaml_data += value_to_yaml("routing", routing_type)
            yaml_data += value_to_yaml("route_expression", config.route_expression)
            yaml_data += value_to_yaml("route_expression_suffix", config.route_expression_suffix)
            yaml_data += dict_to_yaml("servers", config.servers_with_weights)
            yaml_data += dict_to_yaml("server_defaults", config.server_defaults)
            yaml_data += dict_to_yaml("server_ports", config.server_ports)
            yaml_data += dict_to_yaml("server_timeouts", config.server_timeouts)
            yaml_data += dict_to_yaml("server_intervals", config.server_intervals)
            yaml_data += dict_to_yaml("server_retries", config.server_retries)
            yaml_data += list_to_yaml("snmps", config.snmps)
        else:
            return False

        # write to the file
        stream = file(file_name, 'w')
        # Read in the current yaml and write it out to a new file with the current time appended
        # Take the config settings and build a json data string
        # Write the json data to the yaml.dump method using the write enabled stream for the yaml file

        stream.write(yaml_data)
        stream.close()
    except BaseException, e:
        from log import log  # Moved it here to support other things
        log("yaml_write error - %s" % e, "WARNING")

    return True