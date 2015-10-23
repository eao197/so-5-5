headers = Dir[ '**/*.hpp' ].delete_if { |f| /\Wimpl\W/ =~ f }
puts "set( PUBLIC_HEADERS\n  " + headers.join( "\n  " ) + "\n)"

