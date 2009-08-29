#!/bin/sh

# Example playlist script, by Moritz Grimm
# Public Domain where available. Anywhere else: all possible permissions
# are granted and all warranties whatsoever are disclaimed. Use at your
# own risk.
#
# The script plays a simple playlist, but also logs when and which new
# song is queued. Because it writes the entire playlist each time it is
# run, the script will probably not perform well with huge (500+kB)
# playlist files.
#
# Before using it, the configuration variables below should be adjusted
# appropriately.
#
# When using this script in multiple instances of ezstream, use
# different STATE_DIR locations or rename the script to a unique name.


#########################
## BEGIN CONFIGURATION #######################################################
#########################


# STATE_DIR
# Directory where this script keeps state. The directory should only be
# writeable by the user and nobody else (therefore, /tmp is not
# suitable.)
STATE_DIR="."

# PLAYLIST
# The playlist that should be played by this script.
PLAYLIST="playlist.txt"

# LOGFILE (optional)
# File to log song changes to. If not set, logging to file is disabled
# (and this script can be replaced by $PLAYLIST.)
LOGFILE="ezstream.log"

# REPEAT (optional)
# Set to 1 to repeat the playlist indefinitely, set to 0 or leave commented
# out to allow ezstream to determine what to do at end-of-playlist.
#REPEAT=1


#######################
## END CONFIGURATION #########################################################
#######################

_myname="$(basename $0)"

# Check configuration above:
if [ -z "${STATE_DIR}" -o ! -d "${STATE_DIR}" ]; then
	echo "${_myname}: STATE_DIR is not configured, does not exist or is not a directory." >&2
	exit 1
fi
if [ -z "${PLAYLIST}" -o ! -e "${PLAYLIST}" ]; then
	echo "${_myname}: PLAYLIST is not configured or does not exist." >&2
	exit 1
fi
test -n "${REPEAT}" || REPEAT=0

# Set up helper files:
_state="${STATE_DIR}/${_myname}.state"
_playlist="`mktemp "${STATE_DIR}/${_myname}.XXXXXXXXXX"`"
if [ $? -ne 0 ]; then
	echo "${_myname}: Unable to create temporary file." >&2
	exit 1
fi
trap 'rm -f ${_playlist}' 0
trap 'rm -f ${_playlist}; exit 1' 2 15

# Strip comments and empty lines from PLAYLIST, to support .m3u:
sed -e 's,#.*,,g' < ${PLAYLIST} | grep -v '^[[:space:]]*$' >> ${_playlist}
if [ $? -ne 0 ]; then
	echo "${_myname}: Unable to prepare playlist." >&2
	exit 1
fi

# Create state file, if it does not exist:
test -f "${_state}" || touch "${_state}"
if [ $? -ne 0 ]; then
	echo "${_myname}: Unable to create state file." >&2
	exit 1
fi

# Read current track no. from state file:
read _track_no < "${_state}"
if [ -z "${_track_no}" ]; then
	_track_no=1
fi

# Count number of tracks in the playlist:
_num_tracks="$(wc -l < ${_playlist})"
if [ ${_num_tracks} -eq 0 ]; then
	# Nothing to do, really.
	exit 0
fi

# Handle the end-of-playlist case:
if [ ${_track_no} -gt ${_num_tracks} ]; then
	if [ ${REPEAT} -ne 1 ]; then
		# We're done.
		# Output an empty line to indicate end-of-playlist:
		echo
		rm -f "${_state}"
		exit 0
	fi
	_track_no=1
fi

# Get the current track from the playlist:
_track="$(head -n ${_track_no} ${_playlist} | tail -n 1)"

# Output:
echo "${_track}"
test -z "${LOGFILE}" || \
	echo "$(date '+%b %d %H:%M:%S') playlist=\"${PLAYLIST}\" no=${_track_no} track=\"${_track}\"" \
	>> "${LOGFILE}"

# Increment track number and store:
: $((_track_no += 1))
echo "${_track_no}" > "${_state}"

exit 0
