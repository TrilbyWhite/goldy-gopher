
## Gopher

A cli gopher protocol client.

A **minimal** introuction:

Enter comamnds either as positional parameters or at the prompt.  Commands can
be a single letter or the mnemonic "words" for that letter followed by any
parameters for the command.  Tab (completion) will show a list of commands
currently available which are as follows:

* back
	- info: moves back 1 step in the history
* find word1 [ word 2 [ ... ] ]
	- info: run a Veronica search (not implemented yet)
* go host [ path [ port ] ]
	- info: alias to open+show
	- ex: "go sdf.org" or "g floodgap.com"
* help [ keyword ]
	- info: open the help page (for the specified keyword; keywords not
	  implemented yet)
* open host [ path [ port ] ]
	- info: connects to host on port and retrieves the content of path.
	- ex: "open sdf.org" or "o floodgap.com"
* quit
	- info: exit goldy
* show [ start [ stop ] ]
	- info: (re)displays the current content from lines start to stop (inclusive)
	- ex: head: "show 0 10" tail: "show -10 0" (negative offsets not implmented yet)

### Todo
- Implement 'Find' command as an alias to a Veronica search
- Replace auto_show_on_follow with a "max" length and pager integration
- Allow negative numbers as parameters to 'Show' as offset from end
- Write help files
- Customizable download-veiwer: goly will never try to interpret mime-types or
  extensions, but you will be able to specify an external tool that will be
  passed the filename of a downloaded resource.  That external tool may be
  something like xdg-open or an equivalent.
- Download pipes?  An additional parameter to a download command to send the
  incoming data to a user-specified pipe rather than to a file?

## Gainer

Gopher protocol server.

*Caution* This is the first network service software I have written.  There are
no assurances of security.  Please review the code, specifically the
"drop_privileges" function.

### Todo
- A lot.  Please hold feature requests on gainer for now.

## Building

Gold & Gainer depend on libc and libedit.  A Makefile is provided and can be
passed PREFIX (default=/usr) and DESTDIR variables.

* Libc: Builds have only been confirmed against glibc.  Errors building against
  any POSIX compliant libc implementation will be considered bugs - please
  report them.

* Libedit: Can be replaced by GNU readline.  A conditional in the makefile
  allows for `make GPL` to build this variant.  Any redistribution of the
  realine variant would have to be licensed under the GPL.

