#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>

using namespace geode::prelude;

class $modify(MyEditorUI, EditorUI) {

	struct Fields {
		CCMenuItemToggler* m_toggleLinkBtn;
		bool m_playtesting = false;
	};

	static void onModify(auto& self) {
        (void) self.setHookPriority("EditorUI::init", -1500);
    }

	bool init(LevelEditorLayer* lel) {
		if (!EditorUI::init(lel)) {
			return false;
		}
		
		CCSprite* toggleLinkSpriteOn = CCSprite::create("toggleLink-on.png"_spr);
		CCSprite* toggleLinkSpriteOff = CCSprite::create("toggleLink-off.png"_spr);

		m_fields->m_toggleLinkBtn = CCMenuItemToggler::create(toggleLinkSpriteOff, toggleLinkSpriteOn, this, menu_selector(MyEditorUI::onToggleLink));
		m_fields->m_toggleLinkBtn->setID("link-controls-toggle"_spr);
		m_fields->m_toggleLinkBtn->toggle(m_stickyControlsEnabled);
		
		if (CCNode* linkMenu = getChildByID("link-menu")) {
			if (AxisLayout* layout = typeinfo_cast<AxisLayout*>(linkMenu->getLayout())) {
				layout->setGap(0);
			}
			if (CCNode* zoomMenu = getChildByID("zoom-menu")) {
				CCSize winSize = CCDirector::get()->getWinSize();
				linkMenu->setContentHeight(100);
				float origScale = linkMenu->getScale();
				linkMenu->setScale(0.8f * linkMenu->getScale());
				linkMenu->setPositionX(zoomMenu->getPositionX() + 38 * origScale);
				linkMenu->setPositionY(zoomMenu->getPositionY() + 2);
				linkMenu->setAnchorPoint(zoomMenu->getAnchorPoint());
				linkMenu->addChild(m_fields->m_toggleLinkBtn);
				linkMenu->updateLayout();
			}
		}
		
		handleTouchPriority(this);
		forceLinkVisible();
		
		m_linkBtn->setColor({166, 166, 166});
		m_unlinkBtn->setColor({166, 166, 166});
		m_linkBtn->setOpacity(175);
		m_unlinkBtn->setOpacity(175);

		disableLinkButtons(m_stickyControlsEnabled);
		return true;
	}

	void onPlaytest(cocos2d::CCObject* sender) {
        EditorUI::onPlaytest(sender);
        m_fields->m_playtesting = true;
    }

    void onStopPlaytest(cocos2d::CCObject* sender) {
        EditorUI::onStopPlaytest(sender);
        m_fields->m_playtesting = false;
    }

    void onGroupSticky(cocos2d::CCObject* sender) {
        EditorUI::onGroupSticky(sender);
		bool allLinked = verifyLinked(m_selectedObjects);
		if (allLinked) {
			m_unlinkBtn->setColor({255, 255, 255});
			m_unlinkBtn->setOpacity(255);
		}
		m_unlinkBtn->setEnabled(allLinked);
	}


	void keyDown(cocos2d::enumKeyCodes p0) {
		EditorUI::keyDown(p0);
		if (!m_fields->m_playtesting || getChildByID("position-slider")->isVisible()) {
			forceLinkVisible();
			m_fields->m_toggleLinkBtn->toggle(m_stickyControlsEnabled);
		}
		else {
			forceLinkInvisible();
		}
	}

	void forceLinkVisible() {
		m_linkBtn->setVisible(true);
		m_unlinkBtn->setVisible(true);
	}
	void forceLinkInvisible() {
		m_linkBtn->setVisible(false);
		m_unlinkBtn->setVisible(false);
	}

	void disableLinkButtons(bool enabled) {
		m_linkBtn->setEnabled(enabled);
		m_unlinkBtn->setEnabled(enabled);
		if (!enabled) {
			m_linkBtn->setColor({166, 166, 166});
			m_unlinkBtn->setColor({166, 166, 166});
			m_linkBtn->setOpacity(175);
			m_unlinkBtn->setOpacity(175);
		}
		else {
			if (m_selectedObjects->count() > 0) {
				m_linkBtn->setColor({255, 255, 255});
				m_linkBtn->setOpacity(255);
			}
			bool allLinked = verifyLinked(m_selectedObjects);
			if (allLinked) {
				m_unlinkBtn->setColor({255, 255, 255});
				m_unlinkBtn->setOpacity(255);
			}
			m_unlinkBtn->setEnabled(allLinked);
		}
	}

	void onToggleLink(CCObject* obj) {
		toggleStickyControls(!m_stickyControlsEnabled);
		forceLinkVisible();
		disableLinkButtons(m_stickyControlsEnabled);
		GameManager::get()->setGameVariable("0097", m_stickyControlsEnabled);
	}

	bool verifyLinked(cocos2d::CCArray* objects) {

		bool allLinked = false;
		
		if (objects->count() > 1) {
			allLinked = true;
			int linkedGroup = static_cast<GameObject*>(objects->objectAtIndex(0))->m_linkedGroup;
			if (linkedGroup == 0) allLinked = false;
			else {
				for (GameObject* obj : CCArrayExt<GameObject*>(objects)) {
					if (obj->m_linkedGroup != linkedGroup) {
						allLinked = false;
						break;
					}
				}
			}
		}
		return allLinked;
	}

    void showUI(bool show) {
		EditorUI::showUI(show);
		m_fields->m_toggleLinkBtn->setVisible(show);
		if (show) forceLinkVisible();
	}

    void ccTouchEnded(cocos2d::CCTouch* p0, cocos2d::CCEvent* p1) {
		EditorUI::ccTouchEnded(p0, p1);

		bool linked = verifyLinked(m_selectedObjects);
		if (!linked) {
			m_unlinkBtn->setColor({166, 166, 166});
			m_unlinkBtn->setOpacity(175);
		}
		m_unlinkBtn->setEnabled(linked);
		disableLinkButtons(m_stickyControlsEnabled);
	}
};

class $modify(MyEditorPauseLayer, EditorPauseLayer) {

    void onResume(cocos2d::CCObject* sender) {
		EditorPauseLayer::onResume(sender);
		EditorUI* editorUI = EditorUI::get();
		editorUI->m_linkBtn->setVisible(true);
		editorUI->m_unlinkBtn->setVisible(true);
	}
};