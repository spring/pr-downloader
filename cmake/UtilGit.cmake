# This file is part of the Spring engine (GPL v2 or later), see LICENSE.html


#
# Git related CMake utilities
# ---------------------------
#
# Functions and macros defined in this file:
# * Git_Util_Command         - Executes a git command plus arguments.
# * Git_Util_Hash            - Fetches the revision SHA1 hash of the current HEAD.
# * Git_Util_Branch          - Fetches the branch name of the current HEAD.
# * Git_Util_Describe        - Fetches the output of git-describe of the current HEAD.
# * Git_Info                 - Retrieves a lot of info about the HEAD of a repository
#

set(Git_FIND_QUIETLY TRUE)
find_package(Git)

if(GIT_FOUND)

	# Executes a git command plus arguments.
	macro(git_util_command var dir command)
		set(${var})
		set(${var}-NOTFOUND)
		set(CMD_GIT ${GIT_EXECUTABLE} ${command} ${ARGN})
		execute_process(
				COMMAND ${CMD_GIT}
				WORKING_DIRECTORY ${dir}
				RESULT_VARIABLE CMD_RET_VAL
				OUTPUT_VARIABLE ${var}
				ERROR_VARIABLE  GIT_ERROR
				OUTPUT_STRIP_TRAILING_WHITESPACE
				ERROR_STRIP_TRAILING_WHITESPACE)

		if(NOT ${CMD_RET_VAL} EQUAL 0)
			set(${var})
			set(${var}-NOTFOUND "1")
			if(NOT GIT_UTIL_FIND_QUIETLY)
				message(STATUS "Command \"${CMD_GIT}\" in directory ${dir} failed with output:\n\"${GIT_ERROR}\"")
			endif()
		endif()
	endmacro()

	# Fetches the revision SHA1 hash of the current HEAD.
	# This command may fail if dir is not a git repo.
	macro(git_util_hash var dir)
		git_util_command(${var} "${dir}" rev-list -n 1 ${ARGN} HEAD)
	endmacro()

	# Fetches the branch name of the current HEAD.
	# This command may fail if dir is not a git repo.
	# In case dir has a detached HEAD, var will be set to "HEAD",
	# else it will be set to the branch name, eg. "master" or "develop".
	macro(git_util_branch var dir)
		git_util_command(${var} "${dir}" rev-parse --abbrev-ref ${ARGN} HEAD)
	endmacro()

	# Fetches the output of git-describe of the current HEAD.
	# This command may fail if dir is not a git repo.
	# Only tags matching the given pattern (shell glob, see manual for git-tag)
	# may be used.
	# Example tag patterns: all tags:"*", spring-version-tags:"*.*.*"
	macro(git_util_describe var dir tagPattern)
		git_util_command(${var} "${dir}" describe --tags --candidates 999 --match "${tagPattern}" ${ARGN})
	endmacro()

	# Sets the following vars:
	# * ${prefix}_GIT_REVISION_HASH     : `git rev-list -n 1 HEAD`
	#                                     -> 2d6bd9cb5a9ceb9c728dd34a1ab2925d4b0759e0
	# * ${prefix}_GIT_REVISION_NAME     : `git name-rev --name-only --tags --no-undefined --always 2d6bd9cb5a9ceb9c728dd34a1ab2925d4b0759e0`
	#                                     -> 2d6bd9c              # no related tag found (SHA1 starts with 2d6bd9c)
	#                                     -> 0.82.3^0             # exactly 0.82.3
	#                                     -> 0.82.3~2             # 2 commits before 0.82.3
	# * ${prefix}_GIT_DESCRIBE          : `git describe --tags`
	#                                     -> 0.82.3-1776-g2d6bd9c # 1776 commits after 0.82.3 (SHA1 starts with 2d6bd9c)
	#                                     -> 0.82.3               # exactly 0.82.3
	# * ${prefix}_GIT_BRANCH            : `git rev-parse --abbrev-ref HEAD`
	#                                     -> HEAD   # in case we are not on any branch (detached HEAD)
	#                                     -> master # the current branchs name
	# - ${prefix}_GIT_FILES_MODIFIED    : number of uncommitted modified files
	# - ${prefix}_GIT_FILES_ADDED       : number of uncommitted added files
	# - ${prefix}_GIT_FILES_DELETED     : number of uncommitted deleted files
	# - ${prefix}_GIT_FILES_UNVERSIONED : number of uncommitted unversioned files
	# - ${prefix}_GIT_FILES_CLEAN       : TRUE if there are no uncommitted modified files
	# - ${prefix}_GIT_FILES_CLEAN_VERY  : TRUE if there are no uncommitted modified, added, deleted or unversioned files
	macro(git_info dir prefix)

		# Fetch ${prefix}_GIT_REVISION_HASH
		git_util_hash(${prefix}_GIT_REVISION_HASH "${dir}")

		# Fetch ${prefix}_GIT_REVISION_NAME
		set(${prefix}_GIT_REVISION_NAME)
		set(${prefix}_GIT_REVISION_NAME-NOTFOUND)
		if(${prefix}_GIT_REVISION_HASH)
			git_util_command(${prefix}_GIT_REVISION_NAME "${dir}"
					name-rev --name-only --tags --no-undefined --always ${${prefix}_GIT_REVISION_HASH})
		endif()

		# Fetch ${prefix}_GIT_DESCRIBE
		git_util_describe(${prefix}_GIT_DESCRIBE "${dir}" "*")

		# Fetch ${prefix}_GIT_BRANCH
		git_util_branch(${prefix}_GIT_BRANCH "${dir}")

		# Fetch ${prefix}_GIT_FILES_MODIFIED
		# Fetch ${prefix}_GIT_FILES_ADDED
		# Fetch ${prefix}_GIT_FILES_DELETED
		# Fetch ${prefix}_GIT_FILES_UNVERSIONED
		# Fetch ${prefix}_GIT_FILES_CLEAN
		# Fetch ${prefix}_GIT_FILES_CLEAN_VERY
		set(${prefix}_GIT_FILES_MODIFIED)
		set(${prefix}_GIT_FILES_MODIFIED-NOTFOUND)
		set(${prefix}_GIT_FILES_ADDED)
		set(${prefix}_GIT_FILES_ADDED-NOTFOUND)
		set(${prefix}_GIT_FILES_DELETED)
		set(${prefix}_GIT_FILES_DELETED-NOTFOUND)
		set(${prefix}_GIT_FILES_UNVERSIONED)
		set(${prefix}_GIT_FILES_UNVERSIONED-NOTFOUND)
		set(${prefix}_GIT_FILES_CLEAN)
		set(${prefix}_GIT_FILES_CLEAN-NOTFOUND)
		set(${prefix}_GIT_FILES_CLEAN_VERY)
		set(${prefix}_GIT_FILES_CLEAN_VERY-NOTFOUND)

		git_util_command(${prefix}_GIT_STATUS_OUT "${dir}" status --porcelain)

		if(${prefix}_GIT_STATUS_OUT)
			# convert the raw command output to a list like:
			# "M;M;M;M;M;A;D;D;??;??;??"
			string(REGEX REPLACE
					"^[ \t]*([^ \t]+)[ \t]*[^\n\r]+[\n\r]?" "\\1;"
					${prefix}_GIT_STATUS_OUT_LIST_NO_PATHS
					"${${prefix}_GIT_STATUS_OUT}")

			# count ammounts of all modification types in the list
			set(${prefix}_GIT_FILES_MODIFIED    0)
			set(${prefix}_GIT_FILES_ADDED       0)
			set(${prefix}_GIT_FILES_DELETED     0)
			set(${prefix}_GIT_FILES_UNVERSIONED 0)
			foreach(type ${${prefix}_GIT_STATUS_OUT_LIST_NO_PATHS})
				if("${type}" STREQUAL "M")
					math(EXPR ${prefix}_GIT_FILES_MODIFIED    "${${prefix}_GIT_FILES_MODIFIED}    + 1")
				elseif("${type}" STREQUAL "A")
					math(EXPR ${prefix}_GIT_FILES_ADDED       "${${prefix}_GIT_FILES_ADDED}       + 1")
				elseif("${type}" STREQUAL "D")
					math(EXPR ${prefix}_GIT_FILES_DELETED     "${${prefix}_GIT_FILES_DELETED}     + 1")
				elseif("${type}" STREQUAL "??")
					math(EXPR ${prefix}_GIT_FILES_UNVERSIONED "${${prefix}_GIT_FILES_UNVERSIONED} + 1")
				endif()
			endforeach()
			math(EXPR ${prefix}_GIT_FILES_CHANGES
					"${${prefix}_GIT_FILES_MODIFIED} + ${${prefix}_GIT_FILES_ADDED} + ${${prefix}_GIT_FILES_DELETED} + ${${prefix}_GIT_FILES_UNVERSIONED}")

			if(${${prefix}_GIT_FILES_MODIFIED} EQUAL 0)
				set(${prefix}_GIT_FILES_CLEAN TRUE)
			else()
				set(${prefix}_GIT_FILES_CLEAN FALSE)
			endif()
			if(${${prefix}_GIT_FILES_CHANGES} EQUAL 0)
				set(${prefix}_GIT_FILES_CLEAN_VERY TRUE)
			else()
				set(${prefix}_GIT_FILES_CLEAN_VERY FALSE)
			endif()
		else()
			set(${prefix}_GIT_FILES_MODIFIED-NOTFOUND    "1")
			set(${prefix}_GIT_FILES_ADDED-NOTFOUND       "1")
			set(${prefix}_GIT_FILES_DELETED-NOTFOUND     "1")
			set(${prefix}_GIT_FILES_UNVERSIONED-NOTFOUND "1")
			set(${prefix}_GIT_FILES_CLEAN-NOTFOUND       "1")
			set(${prefix}_GIT_FILES_CLEAN_VERY-NOTFOUND  "1")
		endif()
	endmacro()

	# Prints extensive git version info.
	# @see Git_Info
	macro(git_print_info dir)
		set(prefix Git_Print_Info_tmp_prefix_)
		git_info(${dir} ${prefix})
		message("  SHA1              : ${${prefix}_GIT_REVISION_HASH}")
		message("  revision-name     : ${${prefix}_GIT_REVISION_NAME}")
		message("  describe          : ${${prefix}_GIT_DESCRIBE}")
		message("  branch            : ${${prefix}_GIT_BRANCH}")
		message("  local file stats")
		message("    modified:       : ${${prefix}_GIT_FILES_MODIFIED}")
		message("    added:          : ${${prefix}_GIT_FILES_ADDED}")
		message("    deleted:        : ${${prefix}_GIT_FILES_DELETED}")
		message("    unversioned:    : ${${prefix}_GIT_FILES_UNVERSIONED}")
		message("  repository state")
		message("    clean           : ${${prefix}_GIT_FILES_CLEAN}")
		message("    very clean      : ${${prefix}_GIT_FILES_CLEAN_VERY}")
	endmacro()

endif()

