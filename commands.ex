# Vlinder shtrict: very restricted shell
# Copyright (C) 2011   Ronald Landheer-Cieslak
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 3, as 
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.*/

; comments are lines that start with ; or # as the first character
# this line is a comment
; this line is a comment as well

; commands start with the command name, followed by a tab and what the command
; should do. The command name can be up to 40 characters long, the command
; itself can be up to 80 characters long.

; users cannot, at this time, pass arguments to commands

; the commands file should be at /etc/shtrict/commands

; this is the 'ironman' command
ironman	ssh rlc@192.168.200.81
