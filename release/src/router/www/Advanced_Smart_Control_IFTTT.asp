<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>Smart Control</title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<style>
.div_table{
	display:table;
}
.div_tr{
	display:table-row;
}
.div_td{
	display:table-cell;
}
.div_desc{
	position:relative;
	vertical-align:top;
}
.ifff_icon{
	background:url('images/New_ui/icon_ifttt.png') no-repeat;
	height:126px;
	background-size: 90%;
	margin: 31px 0px 0px 74px;
}
</style>
<script>

function initial(){
	show_menu();
	gen_new_pincode();
}

function gen_new_pincode(){
	require(['/require/modules/makeRequest.js'], function(makeRequest){
		makeRequest.start('/get_IFTTTPincode.cgi', refreshPincde, function(){});
	});
}

function refreshPincde(xhr){
	var response = JSON.parse(xhr.responseText);
	document.form.asus_pincode.value = response.ifttt_pincode;
}
</script>
</head>
<body onload="initial();" onunLoad="return unload_body();">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="Advanced_Smart_Control_IFTTT.asp">
<input type="hidden" name="next_page" value="Advanced_Smart_Control_IFTTT.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>" disabled>
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="17">&nbsp;</td>
	<!--=====Beginning of Main Menu=====-->
		<td valign="top" width="202">
			<div id="mainMenu"></div>
			<div id="subMenu"></div>
		</td>
		<td valign="top">
			<div id="tabMenu" class="submenuBlock"></div>
		<!--===================================Beginning of Main Content===========================================-->
			<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
				<tr>
					<td valign="top" >
						<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
							<tbody>
							<tr>
								<td bgcolor="#4D595D" valign="top">
									<div>&nbsp;</div>
									<div class="formfonttitle">Smart Control</div>
									<div id="divSwitchMenu" style="margin-top:-40px;float:right;"><div style="width:110px;height:30px;float:left;border-top-left-radius:8px;border-bottom-left-radius:8px;" class="block_filter"><a href="Advanced_Smart_Control_Alexa.asp"><div class="block_filter_name">Amazon Alexa</div></a></div><div style="width:110px;height:30px;float:left;border-top-right-radius:8px;border-bottom-right-radius:8px;" class="block_filter_pressed"><div class="tab_font_color" style="text-align:center;padding-top:5px;font-size:14px">IFTTT</div></div></div>
									<div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
									<div class="div_table">
											<div class="div_tr">
												<div class="div_td div_desc" style="width:55%">
													<div style="font-weight:bolder;font-size:15px;padding:30px 20px">
														Control ASUS Router via IFTTT
													</div>
													<div style="padding-left:20px;font-style:Arial, Helvetica, sans-serif;font-size:13px;">
														Connect ASUS router to your IFTTT account and use IFTTT applets to control your router. Please 
														<a style="font-weight: bolder;text-decoration: underline;color:#FFCC00" href="https://ifttt.com/login" target="_blank">Login</a>
														 your IFTTT account first and click "GO" to set your IFTTT applets with ASUS router!
													</div>
													<div class="apply_gen" style="padding-top:20px">
														<a href="javascript:" onclick="window.open('https://ifttt.com/asusrouter');" target="_blank">
															<input name="button" type="button" class="button_gen_short" value="GO">
														</a>
													</div>
												</div>
												<div class="div_td">
													<div class="ifff_icon"></div>
												</div>
											</div>
											<div class="div_tr">
												<div style="padding-left:20px;padding-top:36px;width:100%">
													<span style="font-size:15px;">Activation PIN :</span>
													<input class="input_15_table hasDatepicker" style="margin-left:13px;" name="asus_pincode" value="" disabled>
													<span style="text-align:right;font-size:14px;margin-left:16px;cursor:pointer;text-decoration:underline;" onclick="gen_new_pincode();">GEBERATE NEW PIN</span>
												</div>
												<div style="margin-top:16px;padding-left:20px;font-style:Arial, Helvetica, sans-serif;font-size:13px;">
													Activation PIN is used to activate your ASUS router on Amazon Alexa account.This PIN is valid for 2 minutes.
												</div>
											</div>
									</div>
								</td>
							</tr>
							</tbody>
						</table>
					</td>
</form>
				</tr>
			</table>
		<!--===================================Ending of Main Content===========================================-->
		</td>
		<td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>

<div id="footer"></div>
</body>
</html>