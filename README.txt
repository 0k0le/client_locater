Scrape

About:
    Scrape is a framework used to develop web scrapers.
    This example pulls all of the tech gigs from craigslist

Compiling:
    Requirements -
        Make
        libcurl(ssl)
        libpthread
        libssl
        libcrypto
        gcc

    make debug - creates debug build
    make - creates optimized build
    make clean - removes object files and binaries

Run:
    run.sh configfile=dat/config.dat
