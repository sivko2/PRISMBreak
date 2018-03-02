import bb.cascades 1.2
import bb.core 1.2

Sheet 
{
    id: aboutSheet
    
    property int pos: 0
    
	Page
	{	        
	    attachedObjects: 
	        [
/*	            ComponentDefinition 
	            {
	                id: eulaSheetDef
	            	source: "EulaSheet.qml"
	            },
	            ComponentDefinition 
	            {
	                id: privacySheetDef
	            	source: "PrivacySheet.qml"
	            },*/
	            QTimer
	            {
	                id: aboutTimer
	                interval: 500
	                onTimeout:
	                    {
	                        if (pos == 0)
	                        {
	                        	aboutLogo.visible = true;
	                        }
	                        else if (pos == 1)
	                        {
	                        	hwLabel.visible = true;
//	                        	osLabel.visible = true;
	                        }
	                        else if (pos == 2)
	                        {
	                        	footerDivider.visible = true;
	                        	bodyLabel1.visible = true;
//	                        	bodyLabel2.visible = true;
//	                        	bodyLabel3.visible = true;
	                        }
	                        else if (pos == 3)
	                        {
	                        	aboutTimer.stop();
	                        	footerLabel.visible = true;
	                        	bodyButton.visible = true;
	                        }
	                        pos++;
	                    }
	
	            },
	            Invocation 
	            {
	                id: inviteWeb
	                query
	                {
	                    mimeType: "text/html"
	                    uri: "http://prismbreak.pronic.si"
		                invokeActionId: "bb.action.OPEN"                
	                }
	            },
	            Invocation 
	            {
	                id: inviteToDownload
	                query
	                {
	                    mimeType: "text/plain"
	                    data: "Protect your messages with PRISM Break Secure Email! You can download it at " +
	                    	"http://appworld.blackberry.com/webstore/content/49133888"                   
	                }
	            }
	        ]
	
	    titleBar: 
	        TitleBar 
	        {
	     		title: "About"
	     		
	      		acceptAction: 
	     		    ActionItem 
	     		    {
	           			title: "Close"
	           			imageSource: "asset:///images/close.png"
	           			onTriggered: 
	           			    {
	                  			aboutSheet.close();
	                	    	aboutSheet.destroy();
	                  		}
	           		}
	     	}
		        
		ScrollView 
		{
			Container
			{
                topPadding: 20.0
                leftPadding: 20.0
                rightPadding: 20.0
                bottomPadding: 20.0
//                horizontalAlignment: HorizontalAlignment.CenterC
	            
	            ImageView
				{
					id: aboutLogo
					visible: false
					imageSource: "asset:///images/logo.png"
					horizontalAlignment: HorizontalAlignment.Center					
				}
					
				Label
				{
					text: "PRISM Break Secure Email"
					
					textStyle
					{
						base: SystemDefaults.TextStyles.TitleText
						fontWeight: FontWeight.Bold
						color: Color.create("#c32c2b")
					}
					
					horizontalAlignment: HorizontalAlignment.Center
				}
													
				Label
				{
				    id: hwLabel
				    visible: false
					text: "Version: v" + util.version + "   Device: " + util.hardwareInfo + " " + util.hardwareNumber + 
						" " + util.osVersion;
					textStyle
					{
						fontSize: FontSize.XXSmall
						fontWeight: FontWeight.Bold
						color: Color.Black
					}
					
					horizontalAlignment: HorizontalAlignment.Center
				}
				
				Label
				{
				    id: osLabel
				    visible: false
					text: "OS Version: " + util.osVersion;
					textStyle
					{
						fontSize: FontSize.XXSmall
						fontWeight: FontWeight.Bold
						color: Color.Black
					}
					
					horizontalAlignment: HorizontalAlignment.Center
				}
				
				Divider 
				{
				    id: bodyDivider
				    visible: true
	    	    }
				
				Label 
				{
				    id: bodyLabel1
				    visible: false
				    multiline: true
					text: "Ultimate solution to protect your messages!"
					horizontalAlignment: HorizontalAlignment.Fill
					textFormat: TextFormat.Html
					textStyle
					{
						fontWeight: FontWeight.Bold
						textAlign: TextAlign.Center
						fontSize: FontSize.XSmall
					}
				}
				
/*				Label 
				{
				    id: bodyLabel2
				    visible: false
				    multiline: true
					text: "to protect"
					horizontalAlignment: HorizontalAlignment.Fill
					textFormat: TextFormat.Html
					textStyle
					{
						fontWeight: FontWeight.Bold
						textAlign: TextAlign.Center
					}
				}
				
				Label 
				{
				    id: bodyLabel3
				    visible: false
				    multiline: true
					text: "your messages!"
					horizontalAlignment: HorizontalAlignment.Fill
					textFormat: TextFormat.Html
					textStyle
					{
						fontWeight: FontWeight.Bold
						textAlign: TextAlign.Center
						fontSize: FontSize.XSmall
					}
				}*/
				
				Divider 
				{
				    id: footerDivider
				    visible: false
	    	    }
	
				Label
				{
				    id: footerLabel
				    visible: false
					text: "© Copyright Pronic, Meselina Ponikvar Verhovsek s.p. 2014. All rights reserved."
						
					textStyle
					{
						fontSize: FontSize.XXSmall
						fontWeight: FontWeight.Normal
						color: Color.create("#a3a3a3")
					}
					
					textFormat: TextFormat.Html
					horizontalAlignment: HorizontalAlignment.Center
					multiline: true
				}
				
/*				Label 
				{
    			}
				
				Button 
				{
				    id: bodyButton
				    visible: false
					text: "Visit us!"
					horizontalAlignment: HorizontalAlignment.Center
					onClicked: 
					    {
         					inviteWeb.trigger("bb.action.OPEN");
         				}
				}*/
				
			}
		}
    
	actions: 
	    [
	        InvokeActionItem 
	        {
	            id: contactUsAction
	      		title: "Contact Us"
	      		ActionBar.placement: ActionBarPlacement.OnBar
	      		imageSource: "asset:///images/contact_us.png"
	            query
	            {
	                invokeTargetId: "sys.pim.uib.email.hybridcomposer"
	                invokeActionId: "bb.action.SENDEMAIL"
	                uri: "mailto:prismbreak@pronic.si?subject=Help%20about%20PRISM%20Break%20Secure%20Email"
	        	}
	      	},
	        InvokeActionItem 
	        {
	            id: reviewAction
	      		title: "Review App"
	      		ActionBar.placement: ActionBarPlacement.OnBar
	      		imageSource: "asset:///images/review.png"
	            query
	            {
	                invokeTargetId: "sys.appworld"
	                invokeActionId: "bb.action.OPEN"
	                uri: "appworld://content/49133888"
	        	}
	      	}/*,
	        InvokeActionItem 
	        {
	            id: moreAction
	      		title: "More Apps"
	      		ActionBar.placement: ActionBarPlacement.OnBar
	      		imageSource: "asset:///images/moreapps.png"
	            query
	            {
	                invokeTargetId: "sys.appworld"
	                invokeActionId: "bb.action.OPEN"
	                uri: "appworld://vendor/8198"
	            }
	      	},
	        ActionItem 
	        {
	            id: inviteAction
	            title: "Invite to Download"
	            imageSource: "asset:///images/download.png"
	      		ActionBar.placement: ActionBarPlacement.OnBar
	            onTriggered: 
	                {
	                    inviteToDownload.trigger("bb.action.SHARE");
	                }
	      	},
	        ActionItem 
	        {
	            id: policyAction
	            title: "Privacy Policy"
	            imageSource: "asset:///images/policy.png"
	      		ActionBar.placement: ActionBarPlacement.OnBar
	            onTriggered: 
	                {
                 		var privacySheetObj = privacySheetDef.createObject();
                  		privacySheetObj.open();
	                }
	      	},
	        ActionItem 
	        {
	            id: eulaAction
	            title: "License Agreement"
	            imageSource: "asset:///images/eula.png"
	      		ActionBar.placement: ActionBarPlacement.OnBar
	            onTriggered: 
	                {
                 		var eulaSheetObj = eulaSheetDef.createObject();
                  		eulaSheetObj.open();
	                }
	      	}*/
	    ]
	}
	
	onCreationCompleted: 
		{
     		aboutTimer.start();
     	}
}