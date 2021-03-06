#include "OgreMotionState.h"

//-------------------------------------------------------------------------------------
OgreMotionState::OgreMotionState(const btTransform& initialpos, Ogre::SceneNode* visibleObj, bool allowRotation) : mAllowRotation(allowRotation)
{
    mPhysicalPos = initialpos;
    mVisibleObj = visibleObj;
}
//-------------------------------------------------------------------------------------
OgreMotionState::~OgreMotionState(void)
{
}
//-------------------------------------------------------------------------------------
void OgreMotionState::getWorldTransform(btTransform& worldTrans) const
{
    worldTrans = mPhysicalPos;
}
//-------------------------------------------------------------------------------------
void OgreMotionState::setWorldTransform(const btTransform& worldTrans)
{
    if(NULL == mVisibleObj)
        return; // we don't do anything if we don't have a node
    
    // Update our copy of the transform
    mPhysicalPos = worldTrans;
    
    // Copy information from bullet to Ogre
    btVector3 btPos = mPhysicalPos.getOrigin();
    btQuaternion btRot = mPhysicalPos.getRotation();
    this->setPosition(btPos);
    if(mAllowRotation)
        this->setOrientation(btRot);
}
//-------------------------------------------------------------------------------------
void OgreMotionState::setPosition(const btVector3& newPos)
{
    if(NULL == mVisibleObj)
        return; // we don't do anything if we don't have a node
    
    // Update the visible Ogre object's position
    Ogre::Vector3 ogrePos = Ogre::Vector3(newPos.x(), newPos.y(), newPos.z());
    mVisibleObj->setPosition(ogrePos);
}
//-------------------------------------------------------------------------------------
void OgreMotionState::setOrientation(const btQuaternion& newRot)
{
    if(NULL == mVisibleObj)
        return; // we don't do anything if we don't have a node
    
    // Update the visible Ogre object's orientation
    Ogre::Quaternion ogreRot(newRot.w(), newRot.x(), newRot.y(), newRot.z());
    mVisibleObj->setOrientation(ogreRot);
}
//-------------------------------------------------------------------------------------
bool OgreMotionState::allowsRotation(void) const
{
    return mAllowRotation;
}