#include "shaderGen/shaderFeature.h"
#include "shaderGen/shaderOp.h"
#include "shaderGen/featureMgr.h"
#include "materials/materialFeatureTypes.h"
#include "materials/materialFeatureData.h"
#include "gfx/gfxDevice.h"

class ConstantMultFeature : public ShaderFeature
{
public:

   U32 ConstantMultFeature::getOutputTargets(const MaterialFeatureData& fd) const
   {
      return fd.features[MFT_isDeferred] ? ShaderFeature::RenderTarget1 : ShaderFeature::DefaultTarget;
   }

   virtual void processPix( Vector<ShaderComponent*> &componentList, const MaterialFeatureData &fd )
   {
      // Find the constant value
      Var *multConst = (Var *)( LangElement::find("multConst") );
      if( multConst == NULL )
      {
         multConst = new Var;
         multConst->setType( GFX->getAdapterType() == OpenGL ? "vec4" : "float4" );
         multConst->setName( "multConst" );
         multConst->constSortPos = cspPotentialPrimitive;
         multConst->uniform = true;
      }

      ShaderFeature::OutputTarget targ = ShaderFeature::DefaultTarget;
      if (fd.features[MFT_isDeferred])
         targ = ShaderFeature::RenderTarget1;

      // Find output fragment
      Var *color = (Var*) LangElement::find( getOutputTargetVarName(targ) );
      if ( !color )
      {
         color = new Var;
         color->setType( GFX->getAdapterType() == OpenGL ? "vec4" : "fragout" );
         color->setName( getOutputTargetVarName(targ) );

         if(GFX->getAdapterType() != OpenGL)
            color->setStructName( "OUT" );

         output = new GenOp( "   @ = @;", color, multConst );
      }
      output = new GenOp("   @ *= @;\r\n", color, multConst);
   }

   virtual String getName() { return "ConstantMultFeature"; }

   // These methods aren't used
   virtual Var* getVertTexCoord( const String &name ) { return NULL; }
   virtual LangElement *setupTexSpaceMat(  Vector<ShaderComponent*> &componentList, Var **texSpaceMat ) { return NULL; }
   virtual LangElement *expandNormalMap( LangElement *sampleNormalOp, LangElement *normalDecl, LangElement *normalVar, const MaterialFeatureData &fd ) { return NULL; }
   virtual LangElement *assignColor( LangElement *elem, Material::BlendOp blend, LangElement *lerpElem = NULL, ShaderFeature::OutputTarget outputTarget = ShaderFeature::DefaultTarget ) { return NULL; }
};


class ConstMultFeatureReg
{
   bool _handleGFXEvent(GFXDevice::GFXDeviceEventType evt)
   {
      if( evt == GFXDevice::deInit )
      {
         FEATUREMGR->registerFeature( MFT_ConstantMult, new ConstantMultFeature );
      }
      /*else if (evt == GFXDevice::deDestroy)
      {
         FEATUREMGR->unregisterFeature( MFT_ConstantMult );
      }*/
      return true;
   }

public:
   ConstMultFeatureReg()
   {
      GFXDevice::getDeviceEventSignal().notify(this, &ConstMultFeatureReg::_handleGFXEvent, 999.0f);
   }
};

static ConstMultFeatureReg sConstMultFeatureReg;
