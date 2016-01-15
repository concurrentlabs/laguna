#!/usr/bin/env python
#

import datetime
import calendar
from hashlib import sha1
from hashlib import md5
from hmac import new as hmac
import config
from log import log


def token_authenticated(path, arguments, ip):
    passed = False

    # Check to see what type of authentication is needed
    # Currently only supports SeaChange but qualifier added anyway.

    count = 0
    new_dict = {}
    for k, v in arguments.iteritems():
        key = str(k).lower()
        if key == "i" or key == "k" or key == "e" or key == "a" or key == "h":
            count += 1
            new_dict[key] = v[0]  # Since v is a list just pull the only one

    if count == 5 and len(arguments) == count:  # Seachange ALWAYS has 5 parameters
        passed = token_seachange(path, new_dict, ip)

    return passed


def token_seachange(path, argument, ip):
    passed = False
    uri = path  # Just for warnings in the exception handler

    # Rebuild query string in correct order...
    # NOTE: Use more local variables for debugging purposes.

    try:
        # Check for address
        if config.seachange['ip_restrict'] and argument['a'] != ip:
            return False

        # Check for expired date - SeaChange uses UTC
        if config.seachange['expire_date_restrict']:
            if config.token_types['seachange'] == "SHA1":
                expire_date = calendar.timegm(argument['e'])
            else:
                end = str(argument['e'])
                year = end[:4]
                month = end[4:2]
                day = end[6:2]
                hour = end[8:2]
                minute = end[10:2]
                second = end[12:]
                expire_date = datetime.datetime(year, month, day, hour, minute, second)

            if datetime.datetime.utcnow() > expire_date:
                return False

        uri = str("%s?i=%s&k=%s&e=%s&a=%s" % (path, argument['i'], argument['k'], argument['e'], argument['a'])).lower()

        #byte_string = uri.encode('utf-8')
        #byte_string = bytearray(uri, 'utf-8')
        key = config.keys_seachange[argument['k']]

        if config.token_types['seachange'] == "MD5":
            token = hmac(key.decode('hex'), uri, md5)
        else:
            token = hmac(key.decode('hex'), uri, sha1)

        hex_value = token.hexdigest()

        if hex_value == argument['h']:
            passed = True

        if config.logging:
            if passed:
                return_code = config.http_response_codes["found"]
                msg = "%s = %s" % (hex_value, argument['h'])
            else:
                return_code = config.http_response_codes["forbidden"]
                msg = "%s != %s" % (hex_value, argument['h'])
            log("%s - - \"TOKEN(SC) %s\" %s %s \"%s\" \"%s\" \"-\"" % (
                ip, uri, return_code, 0, msg, argument))

    except BaseException, e:
        log("%s - - \"GET %s\" %s %s \"%s\" \"%s\" \"-\"" % (
            ip, uri, config.http_response_codes["forbidden"], 0, e, argument), level="WARNING", log_type="ERROR")

    finally:
        return  passed