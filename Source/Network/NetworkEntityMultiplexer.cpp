#include "NetworkEntityMultiplexer.h"

/**
 * method with the same signature as the Session::onMessage. this
 *   function should be invoked within the session's onMessage method
 *   and forwarded the parameters if the message received by the session
 *   was sent from another {NetworkEntityMultiplexer}.
 *
 * @function   NetworkEntityMultiplexer::onMessage
 *
 * @date       2015-02-28
 *
 * @revision   none
 *
 * @designer   Networking Team
 *
 * @programmer Eric Tsang
 *
 * @note       none
 *
 * @signature  int onMessage(Session* session, Message msg);
 *
 * @param      session session that received the message
 * @param      msg message received from a session object.
 *
 * @return integer indicating the outcome of the operation
 */
int Networking::NetworkEntityMultiplexer::onMessage(Session* session, Message msg)
{
    int* intPtr = (int*) msg.data;
    switch(msg.type)
    {
        case MSG_TYPE_UPDATE:
            networkEntities[*intPtr]->onUpdate(msg);
            break;
        case MSG_TYPE_REGISTER:
            networkEntities[*intPtr]->silentRegister(session);
            networkEntities[*intPtr] = onRegister(*intPtr, *(intPtr+1), session, msg);
            break;
        case MSG_TYPE_UNREGISTER:
            networkEntities[*intPtr]->onUnregister(session, msg);
            networkEntities[*intPtr]->silentUnregister(session);
            networkEntities.erase(*intPtr);
            break;
    }
}
/**
 * should only be called by {NetworkEntity} objects only. it
 *   encapsulates the passed data into a packet, to be sent to all
 *   session objects registered with the {NetworkEntity} associated with
 *   {id}.
 *
 * @function   NetworkEntityMultiplexer::update
 *
 * @date       2015-02-28
 *
 * @revision   none
 *
 * @designer   Networking Team
 *
 * @programmer Eric Tsang
 *
 * @note       none
 *
 * @signature  int update(int id, std::set<Session*>& sessions, Message msg);
 *
 * @param      id identifier associated with a {NetworkEntity} instance
 * @param      sessions set of sessions associated with the network entity that need to be informed of the update
 * @param      msg describes the message to send over the wire
 *
 * @return     integer indicating the result of the operation
 */
int Networking::NetworkEntityMultiplexer::update(int id, std::set<Session*>& sessions, Message msg)
{
    // allocate enough memory to hold message header, and payload
    int datalen = msg.len+sizeof(int);
    char* data = (char*)malloc(datalen);

    // inject header information
    int* intPtr = (int*) &data[0];
    *intPtr = id;

    // inject payload information
    memcpy(&data[sizeof(int)], msg.data, msg.len);

    // create message structure
    Message wireMsg;
    wireMsg.type = MSG_TYPE_UPDATE;
    wireMsg.data = data;
    wireMsg.len  = datalen;

    // send the message to all the sessions
    for(auto it = sessions.begin(); it != sessions.end(); ++it)
    {
        (*it)->send(&wireMsg);
    }

    // free allocated data
    free(data);
}
/**
 * should only be called by the {NetworkEntity} class. it registers the
 *   passed {Session} object with the {NetworkEntity} associated with
 *   {id}, and sends the {msg} to the {session}.
 *
 * @function   NetworkEntityMultiplexer::registerSession
 *
 * @date       2015-02-28
 *
 * @revision   none
 *
 * @designer   Networking Team
 *
 * @programmer Eric Tsang
 *
 * @note       none
 *
 * @signature  int registerSession(int id, int type, Session* session,
 *   Message msg)
 *
 * @param      id identifier associated with a {NetworkEntity} instance
 * @param      type type of entity that's being registered
 * @param      session {Session} to be registered with the
 *   {NetworkEntity} instance
 * @param      msg describes the message to send over the wire. this
 *   message is only sent to the {session}.
 *
 * @return     integer indicating the result of the operation
 */
int Networking::NetworkEntityMultiplexer::registerSession(int id, int type, Session* session, Message msg)
{
    // allocate enough memory to hold message header, and payload
    int datalen = msg.len+sizeof(int)*2;
    char* data = (char*)malloc(datalen);

    // inject header information
    int* intPtr = (int*) &data[0];
    *intPtr = id;
    *(intPtr+1) = type;

    // inject payload information
    memcpy(&data[sizeof(int)*2], msg.data, msg.len);

    // create message structure
    Message wireMsg;
    wireMsg.type = MSG_TYPE_REGISTER;
    wireMsg.data = data;
    wireMsg.len  = datalen;

    // send the message to the sessions
    session->send(&wireMsg);

    // free allocated data
    free(data);
}
/**
 * should only be invoked by the {NetworkEntity} class. it unregisters
 *   the {session} from the {NetworkEntity} instance associated with
 *   {id}, and sends the {msg} to the {session}.
 *
 * @function   NetworkEntityMultiplexer::unregisterSession
 *
 * @date       2015-02-28
 *
 * @revision   none
 *
 * @designer   Networking Team
 *
 * @programmer Eric Tsang
 *
 * @note       none
 *
 * @signature  int unregisterSession(int id, Session* session, Message
 *   msg)
 *
 * @param      id identifier associated with a {NetworkEntity} instance
 * @param      session {Session} to be unregistered with the
 *   {NetworkEntity} instance
 * @param      msg describes the message to send over the wire. this
 *   message is only sent to the {session}.
 *
 * @return     integer indicating the result of the operation
 */
int Networking::NetworkEntityMultiplexer::unregisterSession(int id, Session* session, Message msg)
{
    // allocate enough memory to hold message header, and payload
    int datalen = msg.len+sizeof(int);
    char* data = (char*)malloc(datalen);

    // inject header information
    int* payloadId = (int*) &data[0];
    *payloadId = id;

    // inject payload information
    memcpy(&data[sizeof(int)], msg.data, msg.len);

    // create message structure
    Message wireMsg;
    wireMsg.type = MSG_TYPE_UNREGISTER;
    wireMsg.data = data;
    wireMsg.len  = datalen;

    // send the message to the sessions
    session->send(&wireMsg);

    // free allocated data
    free(data);
}
/**
 * should only be called from within the Networking library. it calls
 *   the update method of the {NetworkEntity} instance associated with
 *   {id}.
 *
 * @function   NetworkEntityMultiplexer::onUpdate
 *
 * @date       2015-02-28
 *
 * @revision   none
 *
 * @designer   Eric Tsang
 *
 * @programmer Eric Tsang
 *
 * @note       none
 *
 * @signature  void onUpdate(int id, Message msg)
 *
 * @param      id identifier associated with a {NetworkEntity} instance.
 * @param      msg describes the message to send over the wire. this
 *   message is only sent to the {session}.
 */
void Networking::NetworkEntityMultiplexer::onUpdate(int id, Message msg)
{
    networkEntities[id]->onUpdate(msg);
}
/**
 * should only be called from within the Networking library. it calls
 *   the onUnregister method of the {NetworkEntity} instance associated
 *   with {id}.
 *
 * @function   NetworkEntityMultiplexer::onUnregister
 *
 * @date       2015-02-28
 *
 * @revision   none
 *
 * @designer   Networking Team
 *
 * @programmer Eric Tsang
 *
 * @note       none
 *
 * @signature  void onUnregister(int id, Session* session, Message msg)
 *
 * @param      id identifier associated with a {NetworkEntity} instance.
 * @param      session session being registered with the NetworkEntity.
 * @param      msg describes the message to send over the wire. this message
 *   is only sent to the {session}.
 */
void Networking::NetworkEntityMultiplexer::onUnregister(int id, Session* session, Message msg)
{
    networkEntities[id]->onUnregister(session,msg);
}
