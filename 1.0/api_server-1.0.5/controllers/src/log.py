#!/usr/bin/env python
#

"""
Logging is implemented here so that the logging api is encapsulated so that we can change the
internal function later to ZeroMQ with a central logging server or cluster. Hadoop could also be used.

The default LEVEL is INFO for the log(...) function.
"""

import logging
import logging.handlers
import os

import config

# One time initialization...
logger = logging.getLogger('api_server')
logger_error = logging.getLogger('api_server_errors')

# If '../' begins then get the current directory
if config is not None:
    if config.log_file.startswith("../"):
        location = os.path.dirname(__file__) + '/'
        location += config.log_file
    else:
        location = config.log_file

    if config.errors_log_file.startswith("../"):
        location_error = os.path.dirname(__file__) + '/'
        location_error += config.errors_log_file
    else:
        location_error = config.errors_log_file

    formatter = logging.Formatter(config.log_format)
    rotation = config.log_rotation
    backups = config.log_backups
    when = config.log_when
    utc = config.log_utc
else:
    utc = False
    when = 'midnight'
    rotation = 24
    backups = 7
    formatter = '%(asctime)s %(name)-12s %(levelname)-8s %(message)s'
    location = os.path.dirname(__file__) + '/../logs/api_server.log'  # This will force config errors to write to this log file at all times.
    location_error = os.path.dirname(__file__) + '/../logs/api_server_errors.log'  # This will force config errors to write to this log file at all times.

# NOTE: TimedRotatingFileHandler can be removed with FileHandler so that the cron can do log rotation.
# Add the log message handler to the logger
if rotation != 0:
    file_handler = logging.handlers.TimedRotatingFileHandler(location, when=when, interval=rotation, backupCount=backups, utc=utc)
    file_handler_error = logging.handlers.TimedRotatingFileHandler(location_error, when=when, interval=rotation, backupCount=backups, utc=utc)
else:
    file_handler = logging.FileHandler(location)
    file_handler_error = logging.FileHandler(location)

file_handler.setFormatter(formatter)
logger.addHandler(file_handler)
logger.setLevel(logging.INFO)

file_handler_error.setFormatter(formatter)
logger_error.addHandler(file_handler_error)
logger_error.setLevel(logging.ERROR)


def log(message, level="INFO", log_type="ACCESS"):
    lvl = level.upper()

    if lvl == "INFO":
        if log_type == "ACCESS":
            logger.info(message)
        else:
            logger_error.info(message)
    elif lvl == "WARNING":
        if log_type == "ACCESS":
            logger.warning(message)
        else:
            logger_error.warning(message)
    elif lvl == "CRITICAL":
        if log_type == "ACCESS":
            logger.critical(message)
        else:
            logger_error.critical(message)
    elif lvl == "DEBUG":
        if log_type == "ACCESS":
            logger.debug(message)
        else:
            logger_error.debug(message)
    elif lvl == "ERROR":
        if log_type == "ACCESS":
            logger.error(message)
        else:
            logger_error.error(message)
