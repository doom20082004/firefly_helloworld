#include "LoginScene.h"
#include "Client.h"
#include "SendMsgScene.h"

Login::~Login() {
}

CCScene* Login::scene() {
    // 'scene' is an autorelease object
    CCScene* scene = CCScene::create();
    
    // 'layer' is an autorelease object
    Login* layer = Login::create();
	
    // add layer as a child to scene
    scene->addChild(layer);
	
    // return the scene
    return scene;
}

bool Login::init() {
    // super init first
    if (!CCLayer::init()) {
        return false;
    }
    
    // surface size
    CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
    CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();
    
    // login item
    CCLabelTTF* label = CCLabelTTF::create("Login", "Helvetica", 40 / CC_CONTENT_SCALE_FACTOR());
    CCMenuItemLabel* loginItem = CCMenuItemLabel::create(label, this, menu_selector(Login::onLoginClicked));
    loginItem->setPosition(ccp(origin.x + visibleSize.width / 2,
                               origin.y + visibleSize.height * 6 / 10));

    // menu
    CCMenu* menu = CCMenu::create(loginItem, NULL);
    menu->setPosition(CCPointZero);
    addChild(menu);
    
    // user name edit
    CCSprite* frame = CCSprite::create("images/textbox_normal.png");
    CCRect rect = CCRectMake(0,
                             0,
                             frame->getContentSize().width,
                             frame->getContentSize().height);
    CCRect insets = CCRectMake(11 / CC_CONTENT_SCALE_FACTOR(),
                               21 / CC_CONTENT_SCALE_FACTOR(),
                               1 / CC_CONTENT_SCALE_FACTOR(),
                               1 / CC_CONTENT_SCALE_FACTOR());
    CCSize editSize = CCSizeMake(visibleSize.width * 4 / 5, 50 / CC_CONTENT_SCALE_FACTOR());
    {
        CCScale9Sprite* editNormal = CCScale9Sprite::create("images/textbox_normal.png",
                                                            rect,
                                                            insets);
        CCScale9Sprite* editPressed = CCScale9Sprite::create("images/textbox_pressed.png",
                                                             rect,
                                                             insets);
        m_usernameEdit = CCEditBox::create(editSize, editNormal, editPressed);
        m_usernameEdit->setReturnType(kKeyboardReturnTypeDone);
        m_usernameEdit->setFontSize(12);
        m_usernameEdit->setText("");
        m_usernameEdit->setFontColor(ccc3(158, 122, 83));
        m_usernameEdit->setMaxLength(8);
        m_usernameEdit->setPosition(ccp(origin.x + visibleSize.width / 2,
                                        origin.y + visibleSize.height * 8 / 10));
        addChild(m_usernameEdit);
    }
    
    // password edit box
    {
        CCScale9Sprite* editNormal = CCScale9Sprite::create("images/textbox_normal.png",
                                                            rect,
                                                            insets);
        CCScale9Sprite* editPressed = CCScale9Sprite::create("images/textbox_pressed.png",
                                                             rect,
                                                             insets);
        m_passwordEdit = CCEditBox::create(editSize, editNormal, editPressed);
        m_passwordEdit->setReturnType(kKeyboardReturnTypeDone);
        m_passwordEdit->setInputFlag(kEditBoxInputFlagPassword);
        m_passwordEdit->setFontColor(ccc3(158, 122, 83));
        m_passwordEdit->setMaxLength(8);
        m_passwordEdit->setFontSize(8);
        m_passwordEdit->setText("");
        m_passwordEdit->setPosition(ccp(origin.x + visibleSize.width / 2,
                                        origin.y + visibleSize.height * 7 / 10));
        addChild(m_passwordEdit);
    }
    
    // connect to server
    CCTCPSocketHub* hub = Client::sharedClient()->getHub();
    hub->registerCallback(1, this);
    hub->createSocket("172.16.96.60", 11009, 1, kCCSocketDefaultTimeout, true);
    addChild(hub);
    
    return true;
}

void Login::onTCPSocketConnected(int tag) {
}

void Login::onTCPSocketDisconnected(int tag) {
    CCLOG("disconnected");
}

void Login::onTCPSocketData(int tag, CCByteBuffer& bb) {
    const CCArray& packets = Client::sharedClient()->addData(bb);
    CCObject* obj = NULL;
    CCARRAY_FOREACH(&packets, obj) {
        Packet* p = (Packet*)obj;
        CCLOG("cmd: %d, data: %s", p->getHeader().command, p->getBody());
        if(p->getHeader().command == Client::LOGIN) {
            CCJSONObject* json = CCJSONObject::create(p->getBody(), p->getBodyLength());
            Client::ErrCode err = (Client::ErrCode)json->optInt("errno");
            if(err != Client::E_OK) {
                string errMsg = json->optString("errmsg");
                CCLOG("error message: %s", errMsg.c_str());
                break;
            } else {
                // remove self from callback
                CCTCPSocketHub* hub = Client::sharedClient()->getHub();
                hub->unregisterCallback(1);
                hub->removeFromParent();
                
                // to send msg scene
                CCDirector::sharedDirector()->replaceScene(SendMsg::scene());
            }
        }
    }
}

void Login::onLoginClicked(CCObject *sender) {
    CCJSONObject* json = CCJSONObject::create();
    json->addString("username", m_usernameEdit->getText());
    json->addString("password", m_passwordEdit->getText());
    Client::sharedClient()->send(1, json, Client::LOGIN);
}