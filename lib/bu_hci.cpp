/**
    Copyright: zirexix 2016

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    The library or code from is restricted to be used in commercial products
    without bunget.cert file. Please obtain a custom cert file from admin@meeiot.org

*/
#include <assert.h>
#include "bybuff.h"
#include "bu_hci.h"
#include "libbungetpriv.h"
#include <iostream>

/****************************************************************************************
*/
bu_hci::bu_hci(SrvDevice* psrv):_pev(psrv),
                                _isDevUp(false),
                                _state(STATE_ZERO),
                                _chekingdev(false),
                                _connected(false)
{
#ifdef ARM_CC
    _delay = 8912;
#else
    _delay = 1024;
#endif
    _socket = new hci_socket_ble(this);
    _aclMtu = 27;
    _aclPendingMax = 3;
}

/****************************************************************************************
*/
bu_hci::~bu_hci()
{
    _clear();
    _socket->close();
    delete _socket;
}

/****************************************************************************************
*/
bool bu_hci::init(int& devid, bool userchannel)
{
    try
    {
        _socket->create();
        if(userchannel)
            _socket->bind_user(&devid);
        else
            _socket->bind_raw(&devid);
//        reset();
//        ::sleep(1);
    }
    catch(hexecption& e)
    {
        e.report();
        return false;
    }
    _devid = devid;
    return check_dev_state();
}

/****************************************************************************************
*/
bool bu_hci::start(int delay)
{
    _delay = delay * 1000;
    bool b =  check_dev_state();
    return b;
}

/****************************************************************************************
*/
void bu_hci::_set_hci_filter()
{
    TRACE(__FUNCTION__);

    struct  _PAACK8
    {
        uint32_t    filter0;
        uint32_t    filter1;
        uint32_t    filter2;
        uint16_t    filter3;
    }  filter =
    {
        btohl((1 << HCI_EVENT_PKT)| (1 << HCI_ACLDATA_PKT)),
#ifdef ACL_MTU_FRAG
        btohl( (1 << EVT_NUM_COMP_PKTS)|
               (1 << EVT_DISCONN_COMPLETE) |
               (1 << EVT_ENCRYPT_CHANGE) |
               (1 << EVT_CMD_COMPLETE) |
               (1 << EVT_CMD_STATUS)),
#else
        btohl( (1 << EVT_DISCONN_COMPLETE) |
               (1 << EVT_ENCRYPT_CHANGE) |
               (1 << EVT_CMD_COMPLETE) |
               (1 << EVT_CMD_STATUS)),
#endif
        btohs(1 << (EVT_LE_META_EVENT - 32)),
        0
    };
#ifdef DEBUG
    bybuff by((const uint8_t*)&filter, sizeof(filter));
    TRACE("[]<=" << by.to_string());
#endif
    _socket->set_filter((const uint8_t*)&filter, sizeof(filter));
}

/****************************************************************************************
*/
void bu_hci::_set_event_mask()
{
    TRACE(__FUNCTION__);
    struct  _PAACK8
    {
        hcihr   _hcihr;
        set_event_mask_cp _set_event_mask_cp;
    }
    event =
    {
        {
            HCI_COMMAND_PKT,
            CMD_OPCODE_PACK(OCF_SET_EVENT_MASK,OGF_HOST_CTL),
            uint8_t(sizeof(set_event_mask_cp)),
        },
        {
            0xff,
            0xff,
            0xfb,
            0xff,
            0x07,
            0xf8,
            0xbf,
            0x3d
        }
    };
    _write_sock(event);
}

/****************************************************************************************
*/
void bu_hci::_set_le_event_mask()
{
    _TRACE(__FUNCTION__);
    struct  _PAACK8
    {
        hcihr   _hcihr;
        set_event_mask_cp _set_event_mask_cp;
    }
    event =
    {
        {
            HCI_COMMAND_PKT,
            CMD_OPCODE_PACK(OCF_SET_EVENT_MASK,OGF_LE_CTL),
            uint8_t(sizeof(set_event_mask_cp)),
        },
        {
            0x1f,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00
        }
    };
    _write_sock(event);
}

/****************************************************************************************
*/
void bu_hci::_read_version()
{
    _TRACE(__FUNCTION__);
    hcihr   cmd =
    {
        HCI_COMMAND_PKT,
        CMD_OPCODE_PACK( OCF_READ_LOCAL_VERSION,OGF_INFO_PARAM),
        0
    };
    _write_sock(cmd);
}

/****************************************************************************************
*/
void bu_hci::_read_baddr()
{
    _TRACE(__FUNCTION__);
    hcihr   cmd =
    {
        HCI_COMMAND_PKT,
         CMD_OPCODE_PACK(OCF_READ_BD_ADDR , OGF_INFO_PARAM),
        0
    };
    _write_sock(cmd);
}

/****************************************************************************************
*/
void bu_hci::_write_le_host()
{
    _TRACE(__FUNCTION__);
    struct  _PAACK8
    {
        hcihr                       hdr;
        write_le_host_supported_cp  str;
    } cmd =
    {
        {
            HCI_COMMAND_PKT,
            CMD_OPCODE_PACK(OCF_WRITE_LE_HOST_SUPPORTED,OGF_HOST_CTL),
            0x2
        },
        {
            0x1,
            0x0
        }
    };
    _write_sock(cmd);
};

/****************************************************************************************
*/
void bu_hci::_read_le_hosts()
{
    _TRACE(__FUNCTION__);
    hcihr cmd =
    {
        HCI_COMMAND_PKT,
        CMD_OPCODE_PACK(OCF_READ_LE_HOST_SUPPORTED,OGF_HOST_CTL),
        0x0
    };
    _write_sock(cmd);
}

/****************************************************************************************
*/
void bu_hci::_set_adv_params(int interval)
{
    _TRACE(__FUNCTION__);
    struct  _PAACK8
    {
        hcihr   _hcihr;
        le_set_advertising_parameters_cp lead;
    } adv =
    {
        {
            HCI_COMMAND_PKT,
            (CMD_OPCODE_PACK(OCF_LE_SET_ADVERTISING_PARAMETERS,OGF_LE_CTL)),
            uint8_t(sizeof(le_set_advertising_parameters_cp))
        }
        ,
        {
            btohs(uint16_t(160)),
            btohs(uint16_t(160)),
            0x00,
            0x00,
            0x00,
            {0x00,0x00,0x00,0x00,0x00,0x00},
            0x07,
            0x00
        }
    };
    _write_sock(adv);
}

/****************************************************************************************
*/
void bu_hci::reset()
{
    _TRACE(__FUNCTION__);
    hcihr    hdr =
    {
        HCI_COMMAND_PKT,
        (CMD_OPCODE_PACK(OCF_RESET, OGF_HOST_CTL)),
        0x00
    };
    _write_sock(hdr);
}

/****************************************************************************************
*/
void bu_hci::set_adv_data(const sdata& data)
{
    _TRACE(__FUNCTION__);
    struct _PAACK8
    {
        hcihr    hdr;
        le_set_advertising_data_cp lea;
    }  lead =
    {
        {
            HCI_COMMAND_PKT,
            (CMD_OPCODE_PACK(OCF_LE_SET_ADVERTISING_DATA,OGF_LE_CTL)),
            32
        },
        {
            (uint8_t)data.len,
            {
                0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0
            }
        }
    };
    ::memcpy(lead.lea.data,data.data,(int)data.len);
    _write_sock(lead);
}

/****************************************************************************************
*/
void bu_hci::set_sca_res_data(const sdata& data)
{
    _TRACE(__FUNCTION__);
    struct  _PAACK8
    {
        hcihr    hdr;
        le_set_scan_response_data_cp lea;
    }lrspd =
    {
        {
            HCI_COMMAND_PKT,
            (CMD_OPCODE_PACK(OCF_LE_SET_SCAN_RESPONSE_DATA,OGF_LE_CTL)),
            32
        },
        {
            (uint8_t)data.len,
            0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0
        }
    };
    ::memcpy(lrspd.lea.data,data.data,(int)data.len);
    _write_sock(lrspd);
}

/****************************************************************************************
*/
void bu_hci::enable_adv(uint8_t enable)
{
    _TRACE(__FUNCTION__);
    struct  _PAACK8
    {
        hcihr   hdr;
        le_set_advertise_enable_cp lsae;

    }sae={
        {
            HCI_COMMAND_PKT,
            (CMD_OPCODE_PACK(OCF_LE_SET_ADVERTISE_ENABLE,OGF_LE_CTL)),
            0x01
        },
        enable
    };
    _write_sock(sae);
    _pev->onAdvertized(enable!=0);
}

/****************************************************************************************
*/
void bu_hci::disconnect(uint16_t handle, uint8_t reason)
{
    _TRACE(__FUNCTION__);
    struct  _PAACK8
    {
        hcihr   hdr;
        disconnect_cp rh;

    }  cmd =
    {
        {
            HCI_COMMAND_PKT,
            (CMD_OPCODE_PACK(OCF_LE_SET_ADVERTISE_ENABLE,OGF_LE_CTL)),
            0x01
        },
        {
            btohs(handle),
            reason
        }
    };
    _write_sock(cmd);
}

/****************************************************************************************
*/
void bu_hci::read_rssi(uint16_t handle)
{
    _TRACE(__FUNCTION__);
    struct  _PAACK8
    {
        hcihr       hdr;
        uint16_t    r;
    } cmd =
    {
        {
            HCI_COMMAND_PKT,
            (CMD_OPCODE_PACK(OCF_READ_RSSI,OGF_STATUS_PARAM)),
            0x02
        },
        btohs(handle)
    };
    _write_sock(cmd);
}


void bu_hci::_le_read_buffer_size()
{
    _TRACE(__FUNCTION__);
    struct _PAACK8
    {
        uint8_t hdr;
        uint16_t cmd;
        uint8_t len;
    }
    packet =
    {
        uint8_t(1),
        CMD_OPCODE_PACK(OCF_LE_READ_BUFFER_SIZE, OGF_LE_CTL),
        uint8_t(0)
    };
    _socket->write(packet);
}


void bu_hci::_read_buffer_size()
{
    _TRACE(__FUNCTION__);
     struct _PAACK8
    {
        uint8_t hdr;
        uint16_t cmd;
        uint8_t len;
    }
    packet =
    {
        uint8_t(0),
        CMD_OPCODE_PACK(OCF_READ_BUFFER_SIZE, OGF_INFO_PARAM),
        uint8_t(0)
    };
    this->_socket->write(packet);
}

/****************************************************************************************
*/
void bu_hci::write_local_name(const char* name)
{
    _TRACE(__FUNCTION__);

    change_local_name_cp cp;
    memset(&cp, 0, sizeof(cp));
    strncpy((char *) cp.name, name, sizeof(cp.name));
    send_cmd(OCF_CHANGE_LOCAL_NAME, OGF_HOST_CTL, CHANGE_LOCAL_NAME_CP_SIZE, &cp);
}

/****************************************************************************************
*/
int bu_hci::read_local_name()
{
    _TRACE(__FUNCTION__);
    read_local_name_rp rp;
    send_cmd(OCF_READ_LOCAL_NAME, OGF_HOST_CTL, READ_LOCAL_NAME_RP_SIZE, &rp);
    return 0;
}

/****************************************************************************************
*/
void bu_hci::send_cmd(uint16_t ocf, uint16_t ogf, uint8_t plen, void *param)
{
    _TRACE(__FUNCTION__);
    uint8_t loco[512];
    hci_command_hdr hc;

    hc.opcode   = CMD_OPCODE_PACK(ocf, ogf);
    hc.plen     = plen;

    memcpy(loco, &hc, sizeof(hc));
    if(plen)
        memcpy(loco+sizeof(hc), param, plen);

    _write_sock(loco, plen+sizeof(hc));
}

/****************************************************************************************
*/
void bu_hci::_onhci_state_chnaged(HCI_STATE state)
{
    _state = state;
}

/****************************************************************************************
*/
bool bu_hci::onSpin(bt_socket* sock) //received
{
    return _pev->onSpin();
}

/****************************************************************************************
*/
bool bu_hci::pool(int loops)
{
    bool rv  = false;

    if(check_dev_state() && _socket->valid())
    {
    	rv = _poolsocket(loops,true);
    }
    else
    {
        rv = _pev->onSpin();
    }
    return rv;
}

/****************************************************************************************
*/
void bu_hci::_clear()
{
    for(auto  &el : _aclIn)
    {
        no_acl_start_len_dynamic* pd = el.second;
        delete pd;
    }
    _aclIn.clear();
    _aclOut.clear();
    _aclPending.clear();
}

/****************************************************************************************
*/
int bu_hci::on_sock_data(uint8_t code, const sdata& buffer) //received
{
    uint8_t  eventType = buffer.data[0];
    uint16_t blen = buffer.len;
    std::string scase="NOT HANDLED ";
    bybuff  trace(buffer.data, buffer.len);
    TRACE("{-->["<< int(buffer.len) <<"]"<< trace.to_string());

    if (HCI_EVENT_PKT == eventType)
    {
		uint8_t  subEventType = buffer.data[1];
        //_TRACE("    Event:" << int(eventType) << ", subevent:" << int(subEventType));

        switch(subEventType)
        {
            case EVT_DISCONN_COMPLETE:
                scase="EVT_DISCONN_COMPLETE";
                {
                    evt_disconn_complete* pdc = (evt_disconn_complete*)(buffer.data+4);
                    pdc->handle = htobs(pdc->handle);
                    memcpy(&_dcached, pdc, sizeof(_dcached));
#ifdef ACL_MTU_FRAG
                    flush_acl();
#endif //ACL_MTU_FRAG
                    _clear();
                    _pev->on_disconnect(pdc);
                    _connected=false;
                }
                break;
            case EVT_ENCRYPT_CHANGE:
                scase="EVT_ENCRYPT_CHANGE";
                {
                    evt_encrypt_change* pec = (evt_encrypt_change*)(buffer.data+4);
                    pec->handle=htobs(pec->handle);
                    _pev->on_encrypt_chnage(pec);
                }
                break;
            case EVT_CMD_COMPLETE:
                scase="       [EVT_CMD_COMPLETE]";
                {
                    no_evt_cmd_complete* necc = (no_evt_cmd_complete*)(buffer.data+3);
                    necc->cmd=htobs(necc->cmd);
                    this->_oncmd_complette(necc);
                }
                break;
            case EVT_LE_META_EVENT:
                scase="EVT_LE_META_EVENT";
                {
                    no_evt_le_meta_event* pev = (no_evt_le_meta_event*)(buffer.data+3);
                    this->_onmeta(pev);
                }
                break;
            case EVT_CONN_REQUEST:
                scase="EVT_CONN_REQUEST";
                {
                    evt_conn_request* preq= (evt_conn_request*)(buffer.data+4);
                    bdaddr_t dest;
                    baswap(&dest, &preq->bdaddr);
                    memcpy(&preq->bdaddr,&dest,sizeof(dest));
                }
                break;
            case  EVT_CMD_STATUS: //OCF_AUTH_REQUESTED
                scase="EVT_CMD_STATUS";
                {
                    evt_cmd_status* pevs = (evt_cmd_status*)(buffer.data+4);
					pevs->opcode = htobs(pevs->opcode);
					uint16_t ogf = CMD_OPCODE_OGF(pevs->opcode);
					uint16_t ocf = CMD_OPCODE_OCF(pevs->opcode);

                    TRACE("CMD_STATUS status:" <<int(pevs->status)<<" ncmd:" <<
												 int(pevs->ncmd) << " opcode(C/G):" <<
												 std::hex<<int(ocf) <<"/"<<int(ogf) << std::dec);
					if(ocf == OCF_EXIT_PERIODIC_INQUIRY)
					{
						//send_cmd(OCF_INQUIRY_CANCEL, OGF_LINK_CTL,0,0);
					}

                }
                break;
            case EVT_REMOTE_NAME_REQ_COMPLETE:
                scase="EVT_REMOTE_NAME_REQ_COMPLETE";
                {
                    evt_remote_name_req_complete* pnc = (evt_remote_name_req_complete*)(buffer.data+4);
                    TRACE("remote name: " << pnc->name);
                }
                break;
#ifdef ACL_MTU_FRAG
        case EVT_NUM_COMP_PKTS:
                scase="EVT_NUM_COMP_PKTS";
                {
                    uint8_t	nhandles = uint8_t(buffer.data[3]);
                   // TRACE("GOT number of completted acl packets:" << int(nhandles));
                    for(uint8_t h=0; h<nhandles; h++)
                    {
                        no_acl_handler_packet* pconfirm = (no_acl_handler_packet*)(buffer.data + 4 + (h*4));
                        pconfirm->handler = htobs(pconfirm->handler);
                        pconfirm->packet = htobs(pconfirm->packet);

                      //  TRACE("GOT Pending handler:" << int(pconfirm->handler) << ", " << int(pconfirm->packet));

                        const auto& ah = _aclPending.find(pconfirm->handler);
                        if(ah == this->_aclPending.end())
                        {
                           // TRACE("HANDLER "<<  int(pconfirm->handler)  <<" NOT FOUND");
                            continue;
                        }
                        else
                        {
                            ah->second -= pconfirm->packet;
                            if(ah->second <= 0)
                            {
                             //   TRACE("DELETE ALL HANDLER packets:" <<",["<<int(pconfirm->handler) <<"]" << int(ah->second));
                                _erase_AclOut(ah->first);
                            }
                        }
                    }
                    this->flush_acl();
        }
        break;
#endif //ACL_MTU_FRAG
            default:
                break;
        }//switch
    }

    else if (HCI_ACLDATA_PKT == eventType)
    {
        uint16_t  val =  oa2t<uint16_t>(buffer.data,1);
        uint16_t flags =  acl_flags(val);
        uint16_t handle = acl_handle(val);

        if(ACL_START_NO_FLUSH == flags)
        {
            //_clear_aclInQueue();  TODO
            scase="ACL_START_NO_FLUSH";
            flags = ACL_START;
        }
        if (ACL_START == flags)
        {
            uint16_t cid  = oa2t<uint16_t>(buffer.data,7);
            uint16_t expectedlen = oa2t<uint16_t>(buffer.data,5);

			uint16_t chunklen = blen-9;
			sdata sd;
			scase="ACL_START";
			sd.len = expectedlen;
			sd.data = buffer.data + 9;
			if (expectedlen == chunklen)
			{
				_pev->on_acl_packet(handle,cid, sd);
			}
			else
			{
                /// assert(_aclIn.find(handle) == _aclIn.end());
				//accumulate data bt handler
                if(_aclIn.find(handle) == _aclIn.end())
				{
					no_acl_start_len_dynamic* pd = new(no_acl_start_len_dynamic);
					pd->cit = (cid);
					pd->len = (chunklen);
					pd->expectedlen = (expectedlen);
					pd->byarr.insert(pd->byarr.end(), sd.data, sd.data + sd.len);
                    _aclIn[handle] = pd;
				}
				else
				{
                    _TRACE("ERROR AHNDLER NOT FOUND ....!. \n" << handle);
				}
			}

        }
        else if (ACL_CONT == flags)
        {
            /// keep accumulating, or discard
            uint16_t chunklen = blen-9;
            sdata sd;

            scase="ACL_CONT";

            auto el = _aclIn.find(handle);
            if (el == _aclIn.end())
                return true;

            no_acl_start_len_dynamic* pd = el->second;
            pd->len += chunklen;
            pd->byarr.insert(pd->byarr.end(), sd.data, sd.data + sd.len);
            //assert(pd->expectedlen == expectedlen);

            if(pd->expectedlen == pd->len )
            {
                sd.len = pd->len;
                sd.data = &pd->byarr[0];
                _pev->on_acl_packet(handle, pd->cit, sd);
                delete pd;
                _aclIn.erase(handle);
            }
        }
        else
        {
            _TRACE("!!! HCI_ACLDATA_PKT  unknown flag: " << std::hex << int(flags) << std::dec);
        }
    }
    else{
       _TRACE("!!!  NO KNOWN on_sock_data EVENTTYPE " << std::hex << int(eventType) << std::dec );
    }
    //TRACE("HCI: " << scase << "    }");
    return true;
}

/****************************************************************************************
*/
void bu_hci::on_error(const hci_error& error)
{
    _TRACE(__FUNCTION__);
    if (error.message == "network-error")
    {
        this->_onhci_state_chnaged(STATE_NETWORK_DOWN);
    }
}

/****************************************************************************************
*/
void bu_hci::_oncmd_complette(const no_evt_cmd_complete* nevcc)
{
    uint16_t    handle;

    switch(nevcc->cmd)
    {
        case RESET_CMD:
             _TRACE("    RESET_CMD");
             reset();
             ::usleep(512000);
            _reconfigure();
            break;
        case WRITE_LE_HOST_SUPPORTED_CMD:
            {
                _TRACE("    cc WRITE_LE_HOST_SUPPORTED_CMD");
                uint8_t le = nevcc->data[0];
                uint8_t simul = nevcc->data[1];
            }
            break;
        case READ_LE_HOST_SUPPORTED_CMD:
            {
                _TRACE("    cc READ_LE_HOST_SUPPORTED_CMD");
                uint8_t le = nevcc->data[0];
                uint8_t simul = nevcc->data[1];
            }
            break;
        case READ_LOCAL_VERSION_CMD:
            {
                _TRACE("    cc READ_LOCAL_VERSION_CMD");
                uint8_t  hciVer = nevcc->data[0];
                if (hciVer < 0x06)
                {
                    this->_onhci_state_chnaged(STATE_UNSUPORTED);
                }
                uint16_t hciRev = oa2t<uint16_t>(nevcc->data,1);
                uint8_t lmpVer = nevcc->data[3];
                uint16_t m1 = oa2t<uint16_t>(nevcc->data,4);
                uint16_t lmpSubVer = oa2t<uint16_t>(nevcc->data,6);

                _pev->on_read_version(hciVer, hciRev, lmpVer, m1, lmpSubVer);
                if (_state != STATE_POWEREDON)
                {
                    enable_adv(0);
                    int interval=160;
                    _pev->le_get_adv_interval(interval);
                    _set_adv_params(interval);
                }
            }
            break;
        case READ_BD_ADDR_CMD:
            {
                _TRACE("    cc READ_BD_ADDR_CMD");
                memcpy(&_address, nevcc->data, sizeof(bdaddr_t));
                _addrtype = ADDR_PUBLIC;
                _pev->on_mac_change(_address);
                char out[32];
                ba2str(&_address, out);
                _TRACE("    BADDR: " << out);
            }
            break;
        case LE_SET_ADVERTISING_PARAMETERS_CMD:
            _TRACE("    cc LE_SET_ADVERTISING_PARAMETERS_CMD");
            _onhci_state_chnaged(STATE_POWEREDON);
            _pev->on_adv_status(_state);
            break;
        case LE_SET_ADVERTISING_DATA_CMD:
            _TRACE("    cc LE_SET_ADVERTISING_DATA_CMD");
            _pev->on_adv_data_status(nevcc->status);
            _TRACE("======= READY TO ACCEPT CONNECTIONS ======");
            break;
        case LE_SET_SCAN_RESPONSE_DATA_CMD:
            _TRACE("    cc LE_SET_SCAN_RESPONSE_DATA_CMD");
            _pev->on_scan_resp_datat_status(nevcc->status);
            break;
        case LE_SET_ADVERTISE_ENABLE_CMD:
            _TRACE("    cc LE_SET_ADVERTISE_ENABLE_CMD");
            _pev->on_adv_enable(nevcc->status);
            _TRACE("======= READY TO ACCEPT CONNECTIONS ======");
            break;
        case READ_RSSI_CMD:
            {
                _TRACE("    cc READ_RSSI_CMD");
                handle =  oa2t<uint16_t>(nevcc->data,0); //result.readUInt16LE(0);
                uint8_t rssi = nevcc->data[2];
                _pev->on_rssi(handle, rssi);
            }
            break;
        case LE_LTK_NEG_REPLY_CMD:
            _TRACE("    cc LE_LTK_NEG_REPLY_CMD");
            handle =  oa2t<uint16_t>(nevcc->data,0);
            _pev->le_ltk_neg_reply(handle);
            break;
        case CMD_OPCODE_PACK(OCF_HOLD_MODE,OGF_LINK_POLICY):
            {
                _TRACE("    cc CMD_OPCODE_PACK(OCF_HOLD_MODE,OGF_LINK_POLICY)");
                hold_mode_cp* hmcp = (hold_mode_cp*)nevcc->data;
            }
            break;
        case CMD_OPCODE_PACK(OCF_INQUIRY,OGF_LINK_CTL):
            {
                _TRACE("    cc CMD_OPCODE_PACK(OCF_INQUIRY,OGF_LINK_CTL)");
            }
            break;
        case CMD_OPCODE_PACK(OCF_LE_READ_BUFFER_SIZE,OGF_LE_CTL):
            {
                _TRACE("    cc CMD_OPCODE_PACK(OCF_LE_READ_BUFFER_SIZE,OGF_LE_CTL)");

                uint16_t mtu = oa2t<uint16_t>(nevcc->data, 0);
                uint16_t maxmtu = oa2t<uint8_t>(nevcc->data, 2);
                if(mtu == 0){
                    _read_buffer_size(); //force a BT buffer because this sucks
                } else {
                    _aclMtu=mtu;
                    _aclPendingMax=maxmtu;
                    TRACE("OCF_LE_READ_BUFFER_SIZE: mtu=" << int(_aclMtu) << ", pendingMax=" << int(maxmtu));
                }
            }
            break;
        case CMD_OPCODE_PACK(OCF_READ_BUFFER_SIZE,OGF_INFO_PARAM):
            {
                 _TRACE("    cc CMD_OPCODE_PACK(OCF_READ_BUFFER_SIZE,OGF_INFO_PARAM)");
                if (nevcc->status==0) {
                    uint16_t mtu = oa2t<uint16_t>(nevcc->data, 0);
                    uint16_t maxmtu = oa2t<uint16_t>(nevcc->data, 3);
                    if(mtu && maxmtu){
                        _aclMtu=mtu;
                        _aclPendingMax=maxmtu;
                        TRACE("OCF_READ_BUFFER_SIZE: mtu=" << int(_aclMtu) << ", pendingMax=" << int(maxmtu));
                    }
                }
            }
            break;
            case CMD_OPCODE_PACK(OCF_SET_EVENT_MASK,OGF_HOST_CTL):
                {
                    bybuff trace;
                    _TRACE("    cc CMD_OPCODE_PACK(OCF_SET_EVENT_MASK,OGF_HOST_CTL)");
                    set_event_mask_cp* p = (set_event_mask_cp*)nevcc->data;
                    trace.append(p->mask, 8);
                    _TRACE("BT-Mask:" << trace.to_string());

                }
            break;
            case CMD_OPCODE_PACK(OCF_SET_EVENT_MASK,OGF_LE_CTL):
                {
                    bybuff trace;
                    _TRACE("    cc CMD_OPCODE_PACK(OCF_SET_EVENT_MASK,OGF_LE_CTL)");
                    set_event_mask_cp* p = (set_event_mask_cp*)nevcc->data;
                    trace.append(p->mask, 8);
                    _TRACE("LE-Mask:" << trace.to_string());

                }
            break;
        default:
            {
                uint16_t ogf = CMD_OPCODE_OGF(nevcc->cmd);
                uint16_t ocf = CMD_OPCODE_OCF(nevcc->cmd);
                _TRACE("    cc UNK command: OCF=" << std::hex <<
                                                int(ocf) <<
                                                ", OGF=" << int(ogf) << std::dec);
            }
            break;
    }
}

/****************************************************************************************
*/
void bu_hci::_onmeta(const no_evt_le_meta_event* leme)
{
    _TRACE(__FUNCTION__);
    if (leme->leMetaEventType == EVT_LE_CONN_COMPLETE)
    {
        _TRACE("mm EVT_LE_CONN_COMPLETE" );
        this->_onle_complette(leme);
        _connected=true;
    }
    else if (leme->leMetaEventType == EVT_LE_CONN_UPDATE_COMPLETE)
    {
        _TRACE("mm EVT_LE_CONN_UPDATE_COMPLETE" );
        this->_onle_con_update_complette(leme);
    }
    else if(leme->leMetaEventType == EVT_LE_ADVERTISING_REPORT)
    {
        le_advertising_info* lead = (le_advertising_info*)leme->data;
        char bsttr[32];

        _TRACE("mm EVT_LE_ADVERTISING_REPORT" );
        ba2str(&lead->bdaddr, bsttr);
        _TRACE("EV TYPE = " << (int)lead->evt_type);
        _TRACE("BADDR_TYPE = " << (int)lead->bdaddr_type);
        _TRACE("ADDR TYPE = " << (int)lead->bdaddr_type);
        _TRACE("MAC = " << (const char*)bsttr);
        _TRACE("LENGTH = " << (int)lead->length);
    }
    else if(leme->leMetaEventType == EVT_LE_LTK_REQUEST)
    {
        _TRACE("mm EVT_LE_LTK_REQUEST" );
    }
    else if(leme->leMetaEventType == EVT_LE_READ_REMOTE_USED_FEATURES_COMPLETE)
    {
        _TRACE("mm EVT_LE_READ_REMOTE_USED_FEATURES_COMPLETE" );
    }
    else
    {
        _TRACE("mm UNKNOWN META EVENT " << std::hex << leme->leMetaEventType << std::dec << std::endl);
    }
}

/****************************************************************************************
*/
void bu_hci::_onle_complette(const no_evt_le_meta_event* leme)
{
    _TRACE(__FUNCTION__);
    uint16_t handle = oa2t<uint16_t>(leme->data,0);
    uint8_t role = leme->data[2];
    HCI_ADDRTYPE addressType = leme->data[3]== 0x01 ? ADDR_RANDOM : ADDR_PUBLIC;
    bdaddr_t addr = *((bdaddr_t*)&leme->data[4]);

    uint16_t interval = oa2t<uint16_t>(leme->data, 10) * 1.25;
    uint16_t latency = oa2t<uint16_t>(leme->data,12); // TODO: multiplier?
    int supervisionTimeout = oa2t<uint16_t>(leme->data, 14) * 10;
    uint8_t  masterClockAccuracy = leme->data[16]; // TODO: multiplier?

    char bsttr[32];
    ba2str(&addr, bsttr);

    TRACE("cc HANDLE = " << (int)handle);
    TRACE("cc role = " << (int)role);
    TRACE("cc address type = " << (int)addressType);
    TRACE("cc address = " << (const char*)bsttr);
    TRACE("cc interval = " << (int)interval);
    TRACE("cc latency = " << (int)latency);
    TRACE("cc supervision timeout = " << (int)supervisionTimeout);
    TRACE("cc master clock accuracy = " << (int)masterClockAccuracy);

    _erase_AclOut(handle);
    _pev->on_le_connected(leme->leMetaEventStatus, handle, role, addressType, addr,
                            interval, latency, supervisionTimeout, masterClockAccuracy);
}

/****************************************************************************************
*/
void bu_hci::_onle_con_update_complette(const no_evt_le_meta_event* leme)
{
    _TRACE(__FUNCTION__);
    uint16_t handle = oa2t<uint16_t>(leme->data,0);
    uint16_t interval = oa2t<uint16_t>(leme->data,2) * 1.25;
    uint16_t latency = oa2t<uint16_t>(leme->data,4); // TODO: multiplier?
    int supervisionTimeout = oa2t<uint16_t>(leme->data,6) * 10;

    TRACE("uu handle = " << handle);
    TRACE("uu interval = " << interval);
    TRACE("uu latency = " << latency);
    TRACE("uu supervision timeout = " << supervisionTimeout);
    _pev->on_le_conn_update_complette_shit(leme->leMetaEventStatus,handle, interval, latency, supervisionTimeout);
}

/****************************************************************************************
*/
bool bu_hci::check_dev_state()
{
    if(_chekingdev){
        return true;
    }
    static int everysec=0;

    _chekingdev=true;
    if(everysec==0)
    {
        bool is_up = _socket->is_up();
        if (_isDevUp != is_up)
        {
            if (is_up)
            {
                try{
                    reset();
                    ::usleep(512000);
                    _reconfigure();
                }catch(...)
                {
                    is_up=false;
                }
            }
            _pev->on_dev_status(is_up);
            _isDevUp = is_up;
        }

        if(_isDevUp==false)
        {
            if(!_recreate_sock())
            {
                _THROW("");
            }
        }
    }
    if(++everysec>400)everysec=0;
    _chekingdev=false;
    return _isDevUp;
}

/****************************************************************************************
*/
bool bu_hci::_recreate_sock()
{
    _socket->close();
    _pev->on_configure_device(_devid);
     try
    {
        _socket->create();
        _socket->bind_raw(&_devid);
    }
    catch(hexecption& e)
    {
        e.report();
        return false;
    }
    return true;
}



/****************************************************************************************
*/
void bu_hci::read_baddr()
{
    this->_read_baddr();
}

/****************************************************************************************
*/
int bu_hci::_poolsocket(int msecs, int callmain)
{
    return _socket->pool(msecs, callmain);
}


/****************************************************************************************
*/
void bu_hci::_reconfigure()
{
    TRACE("==========================");
    TRACE(__FUNCTION__);
    this->_clear();
    this->_set_hci_filter();
    this->_set_event_mask();
    this->_set_le_event_mask();
    _poolsocket(128, false);
    this->_read_version();
    _poolsocket(128, false);
    this->_write_le_host();
    _poolsocket(128, false);
    this->_read_le_hosts();
    _poolsocket(128, false);
    this->_read_baddr();
    _poolsocket(128, false);
    this->_le_read_buffer_size();
    _poolsocket(128, false);
    
}

void bu_hci::enque_acl(uint16_t handle, uint16_t cid, const sdata& sd)
{
    TRACE("::"<<__FUNCTION__);

    bybuff   aclbuf;
    uint16_t hf = (handle | (ACL_START_NO_FLUSH << 12));

#ifdef DEBUG
    //bybuff tr(sd.data, sd.len);
    //TRACE("toQ: handle:" << int(handle) << ", cid: " << int(cid));
    //TRACE("toQ: [" <<int(tr.length())<<"]" << tr.to_string());
#endif // DEBUG

    aclbuf << uint16_t(sd.len);
    aclbuf << uint16_t(cid);
    aclbuf.append(sd.data, sd.len);
    int mtu = int(this->_aclMtu);
    int len = int(aclbuf.length());
    int accum = 0;
    const uint8_t* paclbuff = aclbuf.buffer();

    while(len > 0)
    {
        AclChunk a;
        int      tocpy = std::min(mtu, len);
        a.acl.aclpkt  = uint8_t(HCI_ACLDATA_PKT);
        a.acl.aclhndl = btohs((uint16_t(hf)));
        a.acl.length  = btohs((uint16_t(tocpy)));
        hf |= (ACL_CONT << 12);
        a.aclsz  = 5 + tocpy;
        ::memcpy(a.acl.buff, paclbuff + accum, tocpy);
        accum += tocpy;
        len -= tocpy;
        std::deque<AclChunk>& r = _aclOut[handle];
        r.push_back(a);
      //  TRACE("Adding hander " << int(handle));
    }

   // TRACE("\nin Q: [" <<int(aclbuf.length())<<"]" << aclbuf.to_string());
   // TRACE("Queue: " << _aclOut.size() << " of "<<_aclOut[handle].size() << " chunks");
    flush_acl();
}

void bu_hci::flush_acl()
{
    int handlers = 0;
    for(const auto& hip : _aclPending)
        handlers += hip.second;
    while(handlers < this->_aclPendingMax && _aclOut.size())
    {
        ++handlers;
        write_acl_chunk(_aclOut.begin()->first);
    }
}

void bu_hci::write_acl_chunk(uint16_t handle)
{
    std::deque<AclChunk>& dq = _aclOut[handle];
    if(dq.size())
    {
	TRACE(__FUNCTION__);

        if(_aclPending.find(handle) == _aclPending.end())
            _aclPending[handle]=0;
        _aclPending[handle]++;

        AclChunk& ak = dq.front();
        _write_sock(ak.acl, ak.aclsz);
        dq.pop_front();
        if(dq.size()==0){
            _aclOut.erase(handle);
        }
    }
}

void bu_hci::_erase_AclOut(uint16_t handle)
{
    std::map<uint16_t, std::deque<AclChunk> >::iterator a = _aclOut.find(handle);
    if(a != _aclOut.end())
    {
        _aclOut.erase(handle);
    }

    std::map<uint16_t, int>::iterator it = this->_aclPending.find(handle);
    if(it != this->_aclPending.end())
    {
        this->_aclPending.erase(it);
    }
}

/****************************************************************************************
*/
void bu_hci::write_ack_packet(uint16_t handle, uint16_t cid, const sdata& data)
{
    _TRACE(__FUNCTION__);
    struct  _PAACK8
    {
        struct  _PAACK8
        {
            uint8_t a;
            uint16_t b;
            uint16_t c;
            uint16_t len;
            uint16_t cid;
        } hdr;
        uint8_t  pck[512]; /*enough well see*/
    }
    cmd={
            {
                (uint8_t)HCI_ACLDATA_PKT,
                (uint16_t)(btohs((uint16_t)(handle | (ACL_START_NO_FLUSH << 12)))),
                (uint16_t)(data.len + 4),
                (uint16_t)(btohs((uint16_t)(data.len))),
                (uint16_t)(btohs(cid)),
            },
        {0}
    };
    assert(data.len < 512);
    ::memcpy(cmd.pck, data.data, data.len);
    _write_sock(cmd, data.len + sizeof(cmd.hdr));
}
