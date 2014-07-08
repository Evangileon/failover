int connect_nonblock(struct sockaddr_in sa, int sock, int timeout) {   
    int flags = 0, error = 0, ret = 0;
    fd_set  rset, wset;
    socklen_t   len = sizeof(error);
    struct timeval  ts;
    
    ts.tv_sec = timeout;
    
    FD_ZERO(&rset);
    FD_SET(sock, &rset);
    wset = rset;    //structure assignment ok
    
    if( (flags = fcntl(sock, F_GETFL, 0)) < 0)
        return -1;
    
    if(fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0)
        return -1;
    
    if( (ret = connect(sock, (struct sockaddr *)&sa, 16)) < 0 )
        if (errno != EINPROGRESS)
            return -1;

    if(ret == 0)    //then connect succeeded right away
        goto nonblock_done;
    
    if( (ret = select(sock + 1, &rset, &wset, NULL, (timeout) ? &ts : NULL)) < 0)
        return -1;
    if(ret == 0){   //we had a timeout
        errno = ETIMEDOUT;
        return -1;
    }

    if (FD_ISSET(sock, &rset) || FD_ISSET(sock, &wset)){
        if(getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
            return -1;
    }else
        return -1;

    if(error){  //check if we had a socket error
        errno = error;
        return -1;
    }

nonblock_done:
    if(fcntl(sock, F_SETFL, flags) < 0)
        return -1;

    return 0;
}
