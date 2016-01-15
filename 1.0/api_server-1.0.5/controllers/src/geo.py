#!/usr/bin/env python
#

import config


def geo_restrict(ip):
    passed = False

    if config.geoip is not None and config.geo_country_codes_count > 0:
        country = config.geoip.country_code_by_addr(ip)
        for code in config.geo_country_codes:
            if country == code:
                passed = True
                break
    else:
        passed = True  # This forces the same as not having geo restriction

    return passed

