using UnityEngine.XR.Interaction.Toolkit.Interactables;
using UnityEngine.XR.Interaction.Toolkit.Transformers;
using UnityEngine.XR.Interaction.Toolkit;
using UnityEngine;

public class WallGrabTransformer : XRBaseGrabTransformer
{
     public Boundary boundary;

     /// <inheritdoc />
     protected override RegistrationMode registrationMode => RegistrationMode.SingleAndMultiple;

     /// <inheritdoc />
     public override void OnLink(XRGrabInteractable grabInteractable)
     {
          base.OnLink(grabInteractable);
     }

     /// <inheritdoc />
     public override void Process(XRGrabInteractable grabInteractable, XRInteractionUpdateOrder.UpdatePhase updatePhase, ref Pose targetPose, ref Vector3 localScale)
     {
          Vector3 old = targetPose.position;
          targetPose.position = boundary.clamp(targetPose.position);
          if(old != targetPose.position)
          {
               boundary.showFor(500);
          }
     }
}
