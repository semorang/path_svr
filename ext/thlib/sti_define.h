/* copyright <happyteam@thinkware.co.kr> */
#ifndef _STI_DEFINE_H_
#define _STI_DEFINE_H_

///////////////////////////////////////////////////////////////////////////////
// WINDOW COMMAND �� MESSAGE ����.
// SM�� ���� �޽����� Send Message�� ó��.
///////////////////////////////////////////////////////////////////////////////
// # ����
#define THTRAFFIC_CMD          WM_USER+3     ///< thtraffic cmd
#define THTRAFFIC_MSG          WM_USER+4

#define THTRAFFIC_CMD_NOT_DEFINE      0    ///< cmd not define
#define THTRAFFIC_CMD_INIT              1    ///< traffic ctrl init
#define THTRAFFIC_CMD_ON                2    ///< traffic ctrl on
#define THTRAFFIC_CMD_OFF               3    ///< traffic ctrl off
#define THTRAFFIC_CMD_CLOSE             4    ///< traffic ctrl close
#define THTRAFFIC_CMD_SHARE_RTMLINK        10  ///< RTM Affect Link Shared
#define THTRAFFIC_CMD_ADJUST_STATIC_TIME   11  ///< adjust static display time
// # tpeg (sti_datast.h �� service param�� ��ɼ����� �����.)
//#define THTRAFFIC_CMD_IDIO_DARCGPS_PARSE  101   // idio darcgps data parsing


#define THTRAFFIC_CMD_TCON_CTT            111    ///< tcon ctt req
#define THTRAFFIC_CMD_TCON_RTM            112    ///< tcon rtm req

#define THTRAFFIC_CMD_TPEG_INIT           121     ///< send msg tpeg mw init
#define THTRAFFIC_CMD_TPEG_CLOSE         122     ///< send msg tpeg mw close
#define THTRAFFIC_CMD_TPEG_PLAYABLE      123     ///< send msg tpeg mw playable
#define THTRAFFIC_CMD_TPEG_PLAY           124     ///< send msg tpeg mw play
#define THTRAFFIC_CMD_TPEG_STOP           125     ///< send msg tpeg mw stop
#define THTRAFFIC_CMD_TPEG_CH_CHANNEL    126   // send msg tpeg ä�η� ����.
#define THTRAFFIC_CMD_TPEG_SH_CHANNEL    127    ///< send msg tpeg ä�� ã��.
#define THTRAFFIC_CMD_TPEG_SLEEP_READY    128    ///< send msg SMART�ܸ������� 
  


///////////////////////////////////////////////////////////////////////////////
// THTRAFFIC MSG
// # stictrl���� ���ϵǴ� message. ////////////////////////////////////////////
#define THTRAFFIC_MSG_INIT_SUCCESS  1   ///< traffic ctrl init success
#define THTRAFFIC_MSG_THREAD_ON          2     ///< traffic thread on
#define THTRAFFIC_MSG_THREAD_OFF        3     ///< traffic thread close

#define THTRAFFIC_MSG_CMD_FAIL           4     ///< uniq request FAIL
#define THTRAFFIC_MSG_UPDATE_DATA   5   ///< traffic data update! 
#define THTRAFFIC_MSG_TPEG_SET_STATUS         6     ///< sti update status
#define THTRAFFIC_MSG_TPEG_ADD_EXTRA_DATA    7     ///< sti update add data




///////////////////////////////////////////////////////////////////////////////
// error no ���� => happyway_win.exe���� iNavi.exe�� ���޵Ǵ� Error No!
///////////////////////////////////////////////////////////////////////////////
#define MSG_STI_SUCCESS           (0x00)

#define MSG_STI_WRONG_PARAM              (0x11) /* request parameter is wrong */
#define MSG_STI_CREATE_THREAD_FAIL       (0x12)
#define MSG_STI_FILE_ERROR               (0x13)     /* shared mem or file���� */
#define MSG_STI_INDEX_CACHE_INIT_FAIL (0x14)
#define MSG_STI_NOT_INIT_ERR          (0x15)
#define MSG_STI_CTT_BOUNDARY_ERR        (0x16)      /* ctt index boundary err */

/// tpeg control msg
#define MSG_STI_TPEG_DLL_LOAD_FAIL   (0x20)    /* tpeg_if_dll load fail */
#define MSG_STI_TPEG_FUNC_LOAD_FAIL  (0x21)    /* dll fuction load fail */
#define MSG_STI_TPEG_INIT_FAIL        (0x22)    /* tpeg init func fail */
#define MSG_STI_TPEG_FUNC_FAIL        (0x23)    /* tpeg fuction fail */
#define MSG_STI_TPEG_DIFF_AM          (0x24)    /* �������񽺿� �ٸ� ���ļ� */
#define MSG_STI_TPEG_NO_CHANNEL       (0x25)    /* TPEG ������ ä�� ���� */
#define MSG_STI_TPEG_CH_SEARCHING    (0x26)    /* TPEG ä�� Ž���� */
#define MSG_STI_TPEG_NO_PROVIDER     (0x27)    /* ������ ��ۻ簡 ���� */
#define MSG_STI_TPEG_SERVICE_ERR     (0x28)    /* TPEG ���� error */
#define MSG_STI_TPEG_AUTH_FAIL        (0x29)    /* tpeg authrization fail */
/* tpeg mw msg: iNavi�� stop�� ��û */
#define MSG_STI_TPEG_STOPSERVICE    (0x30)
#define MSG_STI_TPEG_WEAK_SIGNAL    (0x31)  /* tpeg mw msg: ��ȣ�� ���� */
/* tpeg mw msg: TPEG ������ ��ۻ簡 ���� */
#define MSG_STI_TPEG_NO_SERVICE     (0x32)
#define MSG_STI_TPEG_CHANGEDMB_RET  (0x33)  /* tpeg mw msg: ä�� ���� �Ϸ� */
/* tpeg mw msg: ä�� �˻� �Ϸ� */
#define MSG_STI_TPEG_CHANNEL_SEARCH_COMPLETED (0x34)
#define MSG_STI_TPEG_ERROR          (0x35)  /* tpeg mw msg: TPEG error �߻� */
#define MSG_STI_TPEG_PLAY_FAIL      (0x36)  /* tpeg mw play func fail */
#define MSG_STI_TPEG_SUPPORT_FREQ   (0x37)  /* tpeg mw msg: tpeg ���� �˸� */

#if 0
///< idio cdma ������ ���� ����.
#define MSG_STI_IDIO_COM_OPEN_FAIL       (0x20)    /* com ctrl open fail */
#define MSG_STI_IDIO_COM_SETCONF_FAIL   (0x21)    /* com ctrl set config fail */
/* darc log, darc gps log open fail */
#define MSG_STI_IDIO_DARCLOG_OPEN_FAIL  (0x22)
#define MSG_STI_CDMA_REQ_SEND_FAIL    (0x31)
#define MSG_STI_CDMA_RES_RECV_FAIL    (0x32)
#endif



#endif

