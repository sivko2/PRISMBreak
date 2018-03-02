import bb.cascades 1.2

Container 
{
	property alias keyLabel: pubKeyLabel
	
    topPadding: 10.0
    leftPadding: 10.0
    rightPadding: 10.0
    bottomPadding: 10.0

	layout: 
	    DockLayout 
		{
		}
	    
	ImageView 
	{
    	imageSource: "asset:///images/nine_slice.amd"
	    horizontalAlignment: HorizontalAlignment.Fill
	    verticalAlignment: VerticalAlignment.Fill
    }  

	Container 
	{
	    topPadding: 10.0
	    leftPadding: 10.0
	    rightPadding: 10.0
	    bottomPadding: 10.0
//	    background: Color.create(0.85, 0.85, 0.85)
	    horizontalAlignment: HorizontalAlignment.Fill
	             
	    Label
	    {
	    	topMargin: 10
	    	multiline: true
	    	textFormat: TextFormat.Html
	    	textStyle.fontSize: FontSize.XSmall
			text: "High security: <b>Enabled</b>"
		}
	 
	    Label
	    {
	    	topMargin: 10
	    	multiline: true
	    	textFormat: TextFormat.Html
	    	textStyle.fontSize: FontSize.XSmall
			text: "Public/private key type: <b>RSA 1024-bit</b>"
		}
	 
	    Label
	    {
	    	topMargin: 10
	    	multiline: true
	    	textFormat: TextFormat.Html
	    	textStyle.fontSize: FontSize.XSmall
			text: "Public key:"
		}
		 
	    Label
	    {
	        id: pubKeyLabel
	    	multiline: true
	    	textFormat: TextFormat.Html
	    	textStyle.fontSize: FontSize.XSmall
	    	textStyle.color: Color.DarkBlue
			text: crypto.getOwnPublicKeyURL();
		}    
	}
}
