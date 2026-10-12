#include "web_pages.hpp"

namespace smarthome {

// 完整 Web 控制页面（HTML + CSS + JavaScript），用 raw string literal 存储，
// 避免 C 版逐行转义 \n 与 \" 的繁琐，内容与 C 版完全一致。
static const char g_html_page[] = R"html(<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>智能家居控制中心 V3</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:Arial,sans-serif;background:#f0f2f5;color:#333}
.top{background:#2c3e50;color:#fff;padding:12px 20px;display:flex;justify-content:space-between;align-items:center}
.top h1{font-size:18px}
.debug{background:#e74c3c;color:#fff;padding:4px 12px;font-size:12px;border-radius:4px;display:none}
.container{max-width:1000px;margin:0 auto;padding:16px}
.dash{display:flex;gap:12px;margin-bottom:16px}
.dash-card{flex:1;background:#fff;border-radius:10px;padding:14px;box-shadow:0 1px 4px rgba(0,0,0,.08)}
.dash-card.lr{border-left:4px solid #3498db}
.dash-card.br{border-left:4px solid #9b59b6}
.dash-card h3{font-size:13px;color:#666;margin-bottom:6px}
.dash-card .val{font-size:26px;font-weight:bold}
.dash-card .sub{font-size:16px;color:#999;margin-top:4px}
.rooms{display:grid;grid-template-columns:repeat(4,1fr);gap:12px;margin-bottom:16px}
.room{color:#fff;border-radius:12px;padding:30px 12px;text-align:center;cursor:pointer;transition:transform .2s;font-size:16px}
.room:hover{transform:scale(1.03)}
.room.lr{background:linear-gradient(135deg,#3498db,#2980b9)}
.room.br{background:linear-gradient(135deg,#9b59b6,#8e44ad)}
.room.kt{background:linear-gradient(135deg,#e67e22,#d35400)}
.room.bh{background:linear-gradient(135deg,#1abc9c,#16a085)}
.scenes{display:flex;gap:8px;flex-wrap:wrap;margin-bottom:16px}
.scenes button{padding:12px 26px;border:none;border-radius:22px;color:#fff;cursor:pointer;font-size:15px;font-weight:bold}
.scenes button.s{background:#34495e}
.scenes button.m{background:#8e44ad}
.scenes button.a{background:#7f8c8d}
.scenes button.h{background:#27ae60}
.scenes button.o{background:#c0392b}
.panel{display:none;background:#fff;border-radius:10px;padding:16px;box-shadow:0 1px 4px rgba(0,0,0,.08);margin-bottom:16px}
.panel.on{display:block}
.pb{display:flex;align-items:center;margin-bottom:14px;padding-bottom:10px;border-bottom:1px solid #eee}
.pb button{background:#34495e;color:#fff;border:none;padding:6px 14px;border-radius:4px;cursor:pointer;margin-right:12px}
.pb .pt{font-size:17px;font-weight:bold}
.dr{display:flex;align-items:center;justify-content:space-between;padding:10px 0;border-bottom:1px solid #f5f5f5}
.dr:last-child{border:none}
.di{display:flex;align-items:center;gap:10px}
.di .icon{font-size:24px}
.di .name{font-size:14px}
.dc{display:flex;align-items:center;gap:10px}
.sw{position:relative;width:46px;height:24px}
.sw input{opacity:0;width:0;height:0}
.sw .sl{position:absolute;cursor:pointer;top:0;left:0;right:0;bottom:0;background:#ccc;border-radius:24px;transition:.3s}
.sw .sl:before{position:absolute;content:'';height:18px;width:18px;left:3px;bottom:3px;background:#fff;border-radius:50%;transition:.3s}
.sw input:checked+.sl{background:#2ecc71}
.sw input:checked+.sl:before{transform:translateX(22px)}
.slider-row{display:flex;align-items:center;gap:6px}
input[type=range]{width:140px;height:6px;-webkit-appearance:none;background:#ddd;border-radius:3px;outline:none}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:16px;height:16px;background:#3498db;border-radius:50%;cursor:pointer}
.sv{font-size:13px;min-width:38px;text-align:right}
select{padding:4px 8px;border:1px solid #ddd;border-radius:4px;font-size:13px}
.toast{position:fixed;top:70px;left:50%;transform:translateX(-50%);background:rgba(44,62,80,.9);color:#fff;padding:10px 22px;border-radius:6px;font-size:13px;opacity:0;transition:opacity .3s;z-index:999;pointer-events:none}
.toast.on{opacity:1}
@media(max-width:768px){.rooms{grid-template-columns:repeat(2,1fr)}.dash{flex-direction:column}}
</style>
</head>
<body>
<div class="top">
<h1>智能家居控制中心</h1>
<span class="debug" id="dbg">--</span>
</div>
<div class="container">
<div class="dash">
<div class="dash-card lr"><h3>客厅</h3><div class="val"><span id="lr_temp">--</span>°C</div><div class="sub"><span id="lr_humi">--</span>%</div></div>
<div class="dash-card br"><h3>卧室</h3><div class="val"><span id="br_temp">--</span>°C</div><div class="sub"><span id="br_humi">--</span>%</div></div>
</div>
<div id="home" class="panel on">
<div class="rooms">
<div class="room lr">客厅</div>
<div class="room br">卧室</div>
<div class="room kt">厨房</div>
<div class="room bh">卫生间</div>
</div>
<div style="font-weight:bold;margin-bottom:10px">一键场景</div>
<div class="scenes">
<button class="s">睡眠</button>
<button class="m">影院</button>
<button class="a">离家</button>
<button class="h">回家</button>
<button class="o">断电</button>
<button class="z" style="background:#f39c12;padding:8px 18px;border:none;border-radius:18px;color:#fff;cursor:pointer;font-size:14px">设置</button>
</div>
</div>
<div id="lr_panel" class="panel">
<div class="pb"><button id="lr_back">返回</button><div class="pt">客厅</div></div>
<div id="lr_devices"></div>
</div>
<div id="br_panel" class="panel">
<div class="pb"><button id="br_back">返回</button><div class="pt">卧室</div></div>
<div id="br_devices"></div>
</div>
<div id="kt_panel" class="panel">
<div class="pb"><button id="kt_back">返回</button><div class="pt">厨房</div></div>
<div id="kt_devices"></div>
</div>
<div id="bh_panel" class="panel">
<div class="pb"><button id="bh_back">返回</button><div class="pt">卫生间</div></div>
<div id="bh_devices"></div>
</div>
<div id="cfg_panel" class="panel">
<div class="pb"><button id="cfg_back">返回</button><div class="pt">场景自定义设置</div></div>
<div style="margin-bottom:10px"><select id="cfg_scene_dd" onchange="loadCfg()" style="padding:6px 12px;font-size:14px"><option value="0">睡眠模式</option><option value="1">影院模式</option><option value="2">离家模式</option><option value="3">回家模式</option><option value="4">一键断电</option></select></div>
<div id="cfg_content"></div>
</div>
</div>
<div class="toast" id="toast"></div>
<script>
var dbg=document.getElementById('dbg');
dbg.style.display='block';dbg.style.background='#27ae60';dbg.textContent='JS OK';
function api(e,m,b){var x=new XMLHttpRequest();x.open(m,'/api/'+e,true);x.setRequestHeader('Content-Type','application/json');
x.onload=function(){if(x.status!=200){try{var r=JSON.parse(x.responseText);toast(r.message||'failed');}catch(ex){toast('failed');}}};
x.onerror=function(){toast('net error');};x.send(b?JSON.stringify(b):null);}
var tt;function toast(m){var t=document.getElementById('toast');t.textContent=m;t.classList.add('on');clearTimeout(tt);tt=setTimeout(function(){t.classList.remove('on');},1500);}
function refresh(){
var x=new XMLHttpRequest();x.open('GET','/api/status',true);
x.onload=function(){if(x.status!=200)return;try{var d=JSON.parse(x.responseText);
document.getElementById('lr_temp').textContent=d.sensors.lr_temp;
document.getElementById('lr_humi').textContent=d.sensors.lr_humi;
document.getElementById('br_temp').textContent=d.sensors.br_temp;
document.getElementById('br_humi').textContent=d.sensors.br_humi;
var lr=d.livingroom;
if(document.getElementById('lr_light_sw'))document.getElementById('lr_light_sw').checked=lr.light.on;
if(document.getElementById('lr_light_sl'))document.getElementById('lr_light_sl').value=lr.light.brightness;
if(document.getElementById('lr_light_lb'))document.getElementById('lr_light_lb').textContent=lr.light.brightness+'%';
if(document.getElementById('lr_ac_sw'))document.getElementById('lr_ac_sw').checked=lr.ac.on;
if(document.getElementById('lr_ac_sl'))document.getElementById('lr_ac_sl').value=lr.ac.temp;
if(document.getElementById('lr_ac_lb'))document.getElementById('lr_ac_lb').textContent=lr.ac.temp+'°C';
if(document.getElementById('lr_curtain_dd'))document.getElementById('lr_curtain_dd').value=lr.curtain;
if(document.getElementById('lr_strip_sw'))document.getElementById('lr_strip_sw').checked=lr.strip.on;
if(document.getElementById('lr_strip_dd'))document.getElementById('lr_strip_dd').value=lr.strip.color;
if(document.getElementById('lr_tv_sw'))document.getElementById('lr_tv_sw').checked=lr.tv.on;
if(document.getElementById('lr_tv_vol_sl'))document.getElementById('lr_tv_vol_sl').value=lr.tv.volume;
if(document.getElementById('lr_tv_vol_lb'))document.getElementById('lr_tv_vol_lb').textContent=lr.tv.volume;
if(document.getElementById('lr_robot_sw'))document.getElementById('lr_robot_sw').checked=lr.robot;
if(document.getElementById('lr_purifier_sw'))document.getElementById('lr_purifier_sw').checked=lr.purifier.on;
if(document.getElementById('lr_purifier_dd'))document.getElementById('lr_purifier_dd').value=lr.purifier.mode;
if(document.getElementById('lr_speaker_sw'))document.getElementById('lr_speaker_sw').checked=lr.speaker.on;
if(document.getElementById('lr_speaker_vol_sl'))document.getElementById('lr_speaker_vol_sl').value=lr.speaker.volume;
if(document.getElementById('lr_speaker_vol_lb'))document.getElementById('lr_speaker_vol_lb').textContent=lr.speaker.volume;
var br=d.bedroom;
if(document.getElementById('br_light_sw'))document.getElementById('br_light_sw').checked=br.light.on;
if(document.getElementById('br_light_sl'))document.getElementById('br_light_sl').value=br.light.brightness;
if(document.getElementById('br_light_lb'))document.getElementById('br_light_lb').textContent=br.light.brightness+'%';
if(document.getElementById('br_ac_sw'))document.getElementById('br_ac_sw').checked=br.ac.on;
if(document.getElementById('br_ac_sl'))document.getElementById('br_ac_sl').value=br.ac.temp;
if(document.getElementById('br_ac_lb'))document.getElementById('br_ac_lb').textContent=br.ac.temp+'°C';
if(document.getElementById('br_curtain_dd'))document.getElementById('br_curtain_dd').value=br.curtain;
if(document.getElementById('br_strip_sw'))document.getElementById('br_strip_sw').checked=br.strip.on;
if(document.getElementById('br_strip_dd'))document.getElementById('br_strip_dd').value=br.strip.color;
if(document.getElementById('br_speaker_sw'))document.getElementById('br_speaker_sw').checked=br.speaker.on;
if(document.getElementById('br_speaker_vol_sl'))document.getElementById('br_speaker_vol_sl').value=br.speaker.volume;
if(document.getElementById('br_speaker_vol_lb'))document.getElementById('br_speaker_vol_lb').textContent=br.speaker.volume;
if(document.getElementById('br_humidifier_sw'))document.getElementById('br_humidifier_sw').checked=br.humidifier.on;
if(document.getElementById('br_humidifier_dd'))document.getElementById('br_humidifier_dd').value=br.humidifier.level;
if(document.getElementById('br_closet_sw'))document.getElementById('br_closet_sw').checked=br.closet;
if(document.getElementById('br_aroma_sw'))document.getElementById('br_aroma_sw').checked=br.aroma;
var kt=d.kitchen;
if(document.getElementById('kt_light_sw'))document.getElementById('kt_light_sw').checked=kt.light;
if(document.getElementById('kt_outlet_sw'))document.getElementById('kt_outlet_sw').checked=kt.outlet;
if(document.getElementById('kt_fan_sw'))document.getElementById('kt_fan_sw').checked=kt.fan;
if(document.getElementById('kt_fridge_sw'))document.getElementById('kt_fridge_sw').checked=kt.fridge;
if(document.getElementById('kt_microwave_sw'))document.getElementById('kt_microwave_sw').checked=kt.microwave;
if(document.getElementById('kt_dishwasher_sw'))document.getElementById('kt_dishwasher_sw').checked=kt.dishwasher;
if(document.getElementById('kt_oven_sw'))document.getElementById('kt_oven_sw').checked=kt.oven;
if(document.getElementById('kt_purifier_sw'))document.getElementById('kt_purifier_sw').checked=kt.purifier;
var bh=d.bathroom;
if(document.getElementById('bh_light_sw'))document.getElementById('bh_light_sw').checked=bh.light;
if(document.getElementById('bh_fan_sw'))document.getElementById('bh_fan_sw').checked=bh.fan;
if(document.getElementById('bh_heater_sw'))document.getElementById('bh_heater_sw').checked=bh.heater.on;
if(document.getElementById('bh_heater_sl'))document.getElementById('bh_heater_sl').value=bh.heater.temp;
if(document.getElementById('bh_heater_lb'))document.getElementById('bh_heater_lb').textContent=bh.heater.temp+'°C';
if(document.getElementById('bh_bath_heater_sw'))document.getElementById('bh_bath_heater_sw').checked=bh.bath_heater;
if(document.getElementById('bh_mirror_sw'))document.getElementById('bh_mirror_sw').checked=bh.mirror;
if(document.getElementById('bh_toilet_sw'))document.getElementById('bh_toilet_sw').checked=bh.toilet;
}catch(e){console.error(e);}};x.send();}
function nav(p){
var ps=document.querySelectorAll('.panel');for(var i=0;i<ps.length;i++)ps[i].classList.remove('on');
document.getElementById(p).classList.add('on');
if(p!='home')refresh();
}
function makeSw(id,lbl){return'<label class="sw"><input type="checkbox" id="'+id+'_sw"><span class="sl"></span></label>';}
function makeSlider(id,min,max,unit){
return'<div class="slider-row"><input type="range" min="'+min+'" max="'+max+'" id="'+id+'_sl">'+
'<span class="sv" id="'+id+'_lb">--'+unit+'</span></div>';}
function makeDD(id,opts){var h='<select id="'+id+'_dd">';opts.forEach(function(o,i){h+='<option value="'+i+'">'+o+'</option>';});h+='</select>';return h;}
function row(icon,name,html){return'<div class="dr"><div class="di"><span class="icon">'+icon+'</span><span class="name">'+name+'</span></div><div class="dc">'+html+'</div></div>';}
function bindSw(id,dev){
var sw=document.getElementById(id+'_sw');if(!sw)return;
sw.addEventListener('change',function(){
var on=sw.checked;api('device','POST',{device:dev,action:on?'on':'off'});
var sl=document.getElementById(id+'_sl');var lb=document.getElementById(id+'_lb');
if(sl&&on){
var v=80,u='%';
if(dev=='ac'||dev=='br_ac'){v=22;u='°C';}
else if(dev=='heater'){v=40;u='°C';}
else if(dev=='tv'||dev=='speaker'||dev=='br_speaker'){v=50;u='';}
sl.value=v;if(lb)lb.textContent=v+u;}
toast(on?'已开启':'已关闭');});}
function bindSlider(id,dev,unit){
var sl=document.getElementById(id+'_sl');var lb=document.getElementById(id+'_lb');var sw=document.getElementById(id+'_sw');if(!sl)return;
sl.addEventListener('input',function(){
var v=parseInt(sl.value);lb.textContent=v+unit;if(sw)sw.checked=v>0;
var act=v>0?'on':'off';
api('device','POST',{device:dev,action:act,value:v});
});}
function bindDD(id,dev){
var dd=document.getElementById(id+'_dd');if(!dd)return;
dd.addEventListener('change',function(){api('device','POST',{device:dev,action:'set',value:parseInt(dd.value)});toast('已设置');});}
function buildLR(){
var h='';
h+=row('💡','主灯',makeSw('lr_light','light')+makeSlider('lr_light',0,100,'%'));
h+=row('❄️','空调',makeSw('lr_ac','ac')+makeSlider('lr_ac',16,30,'°C')+makeDD('lr_ac_mode',['制冷','制热','送风']));
h+=row('🚪','窗帘',makeDD('lr_curtain',['全开','关左','关右','全关']));
h+=row('🌈','灯带',makeSw('lr_strip','strip')+makeDD('lr_strip_color',['暖光','白光','彩光']));
h+=row('📺','电视',makeSw('lr_tv','tv')+makeSlider('lr_tv_vol',0,100,''));
h+=row('🤖','扫地机器人',makeSw('lr_robot','robot'));
h+=row('🌀','空气净化器',makeSw('lr_purifier','purifier')+makeDD('lr_purifier_mode',['自动','睡眠','强力']));
h+=row('🔊','智能音箱',makeSw('lr_speaker','speaker')+makeSlider('lr_speaker_vol',0,100,''));
document.getElementById('lr_devices').innerHTML=h;
bindSw('lr_light','light');bindSlider('lr_light','light','%');
bindSw('lr_ac','ac');bindSlider('lr_ac','ac','°C');bindDD('lr_ac_mode','ac');
bindDD('lr_curtain','curtain');
bindSw('lr_strip','strip');bindDD('lr_strip_color','strip');
bindSw('lr_tv','tv');bindSlider('lr_tv_vol','tv','');
bindSw('lr_robot','robot');
bindSw('lr_purifier','purifier');bindDD('lr_purifier_mode','purifier');
bindSw('lr_speaker','speaker');bindSlider('lr_speaker_vol','speaker','');
}
function buildBR(){
var h='';
h+=row('💡','卧室主灯',makeSw('br_light','br_light')+makeSlider('br_light',0,100,'%'));
h+=row('❄️','空调',makeSw('br_ac','br_ac')+makeSlider('br_ac',16,30,'°C')+makeDD('br_ac_mode',['制冷','制热','送风']));
h+=row('🚪','窗帘',makeDD('br_curtain',['全开','关左','关右','全关']));
h+=row('🌈','灯带',makeSw('br_strip','br_strip')+makeDD('br_strip_color',['暖光','白光','彩光']));
h+=row('🔊','智能音箱',makeSw('br_speaker','br_speaker')+makeSlider('br_speaker_vol',0,100,''));
h+=row('💨','加湿器',makeSw('br_humidifier','humidifier')+makeDD('br_humidifier_level',['低','中','高']));
h+=row('🚪','衣帽间灯',makeSw('br_closet','closet'));
h+=row('🕯️','香薰机',makeSw('br_aroma','aroma'));
document.getElementById('br_devices').innerHTML=h;
bindSw('br_light','br_light');bindSlider('br_light','br_light','%');
bindSw('br_ac','br_ac');bindSlider('br_ac','br_ac','°C');bindDD('br_ac_mode','br_ac');
bindDD('br_curtain','br_curtain');
bindSw('br_speaker','br_speaker');bindSlider('br_speaker_vol','br_speaker','');
bindSw('br_strip','br_strip');bindDD('br_strip_color','br_strip');
bindSw('br_speaker','br_speaker');
bindSw('br_humidifier','humidifier');bindDD('br_humidifier_level','humidifier');
bindSw('br_closet','closet');
bindSw('br_aroma','aroma');
}
function buildKT(){
var h='';
h+=row('💡','厨房灯',makeSw('kt_light','kt_light'));
h+=row('🧊','制冰机',makeSw('kt_outlet','outlet'));
h+=row('🍳','油烟机',makeSw('kt_fan','kt_fan'));
h+=row('🫕','冰箱',makeSw('kt_fridge','fridge'));
h+=row('📦','微波炉',makeSw('kt_microwave','microwave'));
h+=row('🍽️','洗碗机',makeSw('kt_dishwasher','dishwasher'));
h+=row('🍕','烤箱',makeSw('kt_oven','oven'));
h+=row('💧','净水器',makeSw('kt_purifier','kt_purifier'));
document.getElementById('kt_devices').innerHTML=h;
bindSw('kt_light','kt_light');bindSw('kt_outlet','outlet');bindSw('kt_fan','kt_fan');
bindSw('kt_fridge','fridge');bindSw('kt_microwave','microwave');bindSw('kt_dishwasher','dishwasher');
bindSw('kt_oven','oven');bindSw('kt_purifier','kt_purifier');
}
function buildBH(){
var h='';
h+=row('💡','卫生间灯',makeSw('bh_light','bh_light'));
h+=row('🌀','排风扇',makeSw('bh_fan','bh_fan'));
h+=row('🔥','热水器',makeSw('bh_heater','heater')+makeSlider('bh_heater',30,60,'°C'));
h+=row('🚿','浴霸',makeSw('bh_bath_heater','bath_heater'));
h+=row('💄','镜前灯',makeSw('bh_mirror','mirror'));
h+=row('🚽','智能马桶',makeSw('bh_toilet','toilet'));
document.getElementById('bh_devices').innerHTML=h;
bindSw('bh_light','bh_light');bindSw('bh_fan','bh_fan');
bindSw('bh_heater','heater');bindSlider('bh_heater','heater','°C');
bindSw('bh_bath_heater','bath_heater');bindSw('bh_mirror','mirror');bindSw('bh_toilet','toilet');
}
document.querySelector('.room.lr').addEventListener('click',function(){if(!document.getElementById('lr_light_sw'))buildLR();nav('lr_panel');});
document.querySelector('.room.br').addEventListener('click',function(){if(!document.getElementById('br_light_sw'))buildBR();nav('br_panel');});
document.querySelector('.room.kt').addEventListener('click',function(){if(!document.getElementById('kt_light_sw'))buildKT();nav('kt_panel');});
document.querySelector('.room.bh').addEventListener('click',function(){if(!document.getElementById('bh_light_sw'))buildBH();nav('bh_panel');});
document.getElementById('lr_back').addEventListener('click',function(){nav('home');});
document.getElementById('br_back').addEventListener('click',function(){nav('home');});
document.getElementById('kt_back').addEventListener('click',function(){nav('home');});
document.getElementById('bh_back').addEventListener('click',function(){nav('home');});
document.getElementById('cfg_back').addEventListener('click',function(){nav('home');});
function mkRow(lbl,id,min,max,unit){return'<div class="dr"><span style="font-size:13px;color:#555">'+lbl+'</span><div class="slider-row" style="width:180px"><input type="range" min="'+min+'" max="'+max+'" id="'+id+'"><span class="sv" id="'+id+'_v">--'+unit+'</span></div></div>';}
function mkDd(lbl,id,opts){var s='<div class="dr"><span style="font-size:13px;color:#555">'+lbl+'</span><select id="'+id+'" style="padding:4px 8px">';opts.forEach(function(o,i){s+='<option value="'+i+'">'+o+'</option>';});s+='</select></div>';return s;}
function mkDd2(id,opts){var s='<select id="'+id+'" style="padding:4px 8px">';opts.forEach(function(o,i){s+='<option value="'+i+'">'+o+'</option>';});s+='</select>';return s;}
function mkSwCfg(lbl,id){return'<div class="dr"><span style="font-size:13px;color:#555">'+lbl+'</span><label class="sw"><input type="checkbox" id="'+id+'"><span class="sl"></span></label></div>';}
function mkSw2(id){return'<label class="sw"><input type="checkbox" id="'+id+'"><span class="sl"></span></label>';}
function mkDev(lbl,html){return'<div style="padding:8px 0;border-bottom:1px solid #f0f0f0;display:flex;align-items:center;gap:8px;flex-wrap:wrap"><span style="font-weight:bold;font-size:13px;min-width:55px">'+lbl+'</span>'+html+'</div>';}
function buildCfg(){
var h='<b>客厅</b><br>';
h+=mkDev('主灯',mkSw2('cfg_llo')+mkRow('亮度','cfg_lrb',0,100,'%'));
h+=mkDev('空调',mkSwCfg('','cfg_lao')+mkRow('','cfg_lat',16,30,'°C')+mkDd2('cfg_lam',['制冷','制热','送风']));
h+=mkDev('窗帘',mkDd2('cfg_lcu',['全开','关左','关右','全关']));
h+=mkDev('灯带',mkSwCfg('','cfg_lso')+mkDd2('cfg_lsc',['暖光','白光','彩光']));
h+=mkDev('电视',mkSw2('cfg_lto')+mkRow('','cfg_ltv',0,100,'音量'));
h+=mkDev('扫地机',mkSwCfg('','cfg_lro'));
h+=mkDev('净化器',mkSwCfg('','cfg_lpo')+mkDd2('cfg_lpm',['自动','睡眠','强力']));
h+=mkDev('音箱',mkSw2('cfg_lsp2')+mkRow('','cfg_lsp',0,100,'音量'));
h+='<br><b>卧室</b><br>';
h+=mkDev('主灯',mkSw2('cfg_blo')+mkRow('亮度','cfg_brb',0,100,'%'));
h+=mkDev('空调',mkSwCfg('','cfg_bao')+mkRow('','cfg_bat',16,30,'°C')+mkDd2('cfg_bam',['制冷','制热','送风']));
h+=mkDev('窗帘',mkDd2('cfg_bcu',['全开','关左','关右','全关']));
h+=mkDev('灯带',mkSwCfg('','cfg_bso')+mkDd2('cfg_bsc',['暖光','白光','彩光']));
h+=mkDev('音箱',mkSw2('cfg_bsp2')+mkRow('','cfg_bsp',0,100,'音量'));
h+=mkDev('加湿器',mkSwCfg('','cfg_bho')+mkDd2('cfg_bhl',['低','中','高']));
h+=mkDev('衣帽间',mkSwCfg('','cfg_bco'));
h+=mkDev('香薰机',mkSwCfg('','cfg_bar'));
h+='<br><b>厨房</b><br>';
var ktSw=[['厨房灯','cfg_ktl'],['制冰机','cfg_kto'],['油烟机','cfg_ktf'],['冰箱','cfg_ktr'],['微波炉','cfg_ktm'],['洗碗机','cfg_ktd'],['烤箱','cfg_kte'],['净水器','cfg_ktp']];
for(var i=0;i<ktSw.length;i++)h+=mkSwCfg(ktSw[i][0],ktSw[i][1]);
h+='<br><b>卫生间</b><br>';
h+=mkDev('热水器',mkSw2('cfg_bhh')+mkRow('水温','cfg_bht',30,60,'°C'));
var bhSw=[['卫生间灯','cfg_bhl2'],['排风扇','cfg_bhf'],['浴霸','cfg_bhb'],['镜前灯','cfg_bhm'],['智能马桶','cfg_bht2']];
for(var i=0;i<bhSw.length;i++)h+=mkSwCfg(bhSw[i][0],bhSw[i][1]);
document.getElementById('cfg_content').innerHTML=h;
var slIds=['cfg_lrb','cfg_lat','cfg_ltv','cfg_lsp','cfg_brb','cfg_bat','cfg_bsp','cfg_bht'];
var slUnits=['%','°C','','','%','°C','','°C'];
for(var i=0;i<slIds.length;i++){(function(id,u){
var sl=document.getElementById(id);var lb=document.getElementById(id+'_v');if(!sl)return;
sl.addEventListener('input',function(){lb.textContent=this.value+u;saveCfg();});
})(slIds[i],slUnits[i]);}
var ddIds=['cfg_lam','cfg_lcu','cfg_lsc','cfg_lpm','cfg_bam','cfg_bcu','cfg_bsc','cfg_bhl'];
for(var i=0;i<ddIds.length;i++)(function(id){var dd=document.getElementById(id);if(dd)dd.addEventListener('change',function(){saveCfg();});})(ddIds[i]);
var swIds=['cfg_llo','cfg_lao','cfg_lso','cfg_lto','cfg_lro','cfg_lpo','cfg_lsp2','cfg_blo','cfg_bao','cfg_bso','cfg_bsp2','cfg_bho','cfg_bco','cfg_bar','cfg_ktl','cfg_kto','cfg_ktf','cfg_ktr','cfg_ktm','cfg_ktd','cfg_kte','cfg_ktp','cfg_bhl2','cfg_bhf','cfg_bhh','cfg_bhb','cfg_bhm','cfg_bht2'];
for(var i=0;i<swIds.length;i++)(function(id){var sw=document.getElementById(id);if(sw)sw.addEventListener('change',function(){saveCfg();});})(swIds[i]);
}
var _cfgReady=false;
function loadCfg(){
_cfgReady=false;
var x=new XMLHttpRequest();x.open('GET','/api/scene_cfg',true);x.onload=function(){
if(x.status!=200)return;
try{var sc=JSON.parse(x.responseText);var i=parseInt(document.getElementById('cfg_scene_dd').value);var c=sc[i];
var s=function(id,val,u){var sl=document.getElementById(id);var lb=document.getElementById(id+'_v');if(sl)sl.value=val;if(lb)lb.textContent=val+u;};
s('cfg_lrb',c.lr_light_brightness,'%');s('cfg_lat',c.lr_ac_temp,'°C');s('cfg_ltv',c.lr_tv_volume,'');
s('cfg_lsp',c.lr_speaker_volume,'');s('cfg_brb',c.br_light_brightness,'%');s('cfg_bat',c.br_ac_temp,'°C');
s('cfg_bsp',c.br_speaker_volume,'');s('cfg_bht',c.bh_heater_temp,'°C');
var d=function(id,val){var dd=document.getElementById(id);if(dd)dd.value=val;};
d('cfg_lam',c.lr_ac_mode);d('cfg_lcu',c.lr_curtain_state);d('cfg_lsc',c.lr_strip_color);d('cfg_lpm',c.lr_purifier_mode);
d('cfg_bam',c.br_ac_mode);d('cfg_bcu',c.br_curtain_state);d('cfg_bsc',c.br_strip_color);d('cfg_bhl',c.br_humidifier_level);
var w=function(id,val){var sw=document.getElementById(id);if(sw)sw.checked=val;};
w('cfg_llo',c.lr_light_on);w('cfg_lao',c.lr_ac_on);w('cfg_lso',c.lr_strip_on);
w('cfg_lto',c.lr_tv_on);w('cfg_lro',c.lr_robot_on);w('cfg_lpo',c.lr_purifier_on);w('cfg_lsp2',c.lr_speaker_on);
w('cfg_blo',c.br_light_on);w('cfg_bao',c.br_ac_on);w('cfg_bso',c.br_strip_on);w('cfg_bsp2',c.br_speaker_on);
w('cfg_bho',c.br_humidifier_on);w('cfg_bco',c.br_closet_light_on);w('cfg_bar',c.br_aroma_on);
w('cfg_ktl',c.kt_light_on);w('cfg_kto',c.kt_outlet_on);w('cfg_ktf',c.kt_fan_on);w('cfg_ktr',c.kt_fridge_on);
w('cfg_ktm',c.kt_microwave_on);w('cfg_ktd',c.kt_dishwasher_on);w('cfg_kte',c.kt_oven_on);w('cfg_ktp',c.kt_purifier_water_on);
w('cfg_bhl2',c.bh_light_on);w('cfg_bhf',c.bh_fan_on);w('cfg_bhh',c.bh_heater_on);
w('cfg_bhb',c.bh_bath_heater_on);w('cfg_bhm',c.bh_mirror_light_on);w('cfg_bht2',c.bh_smart_toilet_on);
}catch(e){}_cfgReady=true;}
x.send();}
var _st;function saveCfg(){
if(!_cfgReady)return;
clearTimeout(_st);_st=setTimeout(function(){
var i=parseInt(document.getElementById('cfg_scene_dd').value);
var d={scene:i};
var g=function(id){var sl=document.getElementById(id);return sl?parseInt(sl.value):0;};
var h=function(id){var dd=document.getElementById(id);return dd?parseInt(dd.value):0;};
var c=function(id){var sw=document.getElementById(id);return sw?sw.checked?1:0:0;};
d.lr_light_brightness=g('cfg_lrb');d.lr_ac_temp=g('cfg_lat');d.lr_tv_volume=g('cfg_ltv');
d.lr_speaker_volume=g('cfg_lsp');d.br_light_brightness=g('cfg_brb');d.br_ac_temp=g('cfg_bat');
d.br_speaker_volume=g('cfg_bsp');d.bh_heater_temp=g('cfg_bht');
d.lr_ac_mode=h('cfg_lam');d.lr_curtain_state=h('cfg_lcu');d.lr_strip_color=h('cfg_lsc');d.lr_purifier_mode=h('cfg_lpm');
d.br_ac_mode=h('cfg_bam');d.br_curtain_state=h('cfg_bcu');d.br_strip_color=h('cfg_bsc');d.br_humidifier_level=h('cfg_bhl');
d.lr_light_on=c('cfg_llo');d.lr_ac_on=c('cfg_lao');d.lr_strip_on=c('cfg_lso');
d.lr_tv_on=c('cfg_lto');d.lr_robot_on=c('cfg_lro');d.lr_purifier_on=c('cfg_lpo');d.lr_speaker_on=c('cfg_lsp2');
d.br_light_on=c('cfg_blo');d.br_ac_on=c('cfg_bao');d.br_strip_on=c('cfg_bso');d.br_speaker_on=c('cfg_bsp2');
d.br_humidifier_on=c('cfg_bho');d.br_closet_light_on=c('cfg_bco');d.br_aroma_on=c('cfg_bar');
d.kt_light_on=c('cfg_ktl');d.kt_outlet_on=c('cfg_kto');d.kt_fan_on=c('cfg_ktf');d.kt_fridge_on=c('cfg_ktr');
d.kt_microwave_on=c('cfg_ktm');d.kt_dishwasher_on=c('cfg_ktd');d.kt_oven_on=c('cfg_kte');d.kt_purifier_water_on=c('cfg_ktp');
d.bh_light_on=c('cfg_bhl2');d.bh_fan_on=c('cfg_bhf');d.bh_heater_on=c('cfg_bhh');
d.bh_bath_heater_on=c('cfg_bhb');d.bh_mirror_light_on=c('cfg_bhm');d.bh_smart_toilet_on=c('cfg_bht2');
api('scene_cfg','POST',d);
},300);}
document.querySelector('button.s').addEventListener('click',function(){api('scene','POST',{scene:'sleep'});toast('睡眠模式已启动');setTimeout(refresh,500);});
document.querySelector('button.m').addEventListener('click',function(){api('scene','POST',{scene:'movie'});toast('影院模式已启动');setTimeout(refresh,500);});
document.querySelector('button.a').addEventListener('click',function(){api('scene','POST',{scene:'away'});toast('离家模式已启动');setTimeout(refresh,500);});
document.querySelector('button.h').addEventListener('click',function(){api('scene','POST',{scene:'home'});toast('回家模式已启动');setTimeout(refresh,500);});
document.querySelector('button.o').addEventListener('click',function(){api('scene','POST',{scene:'off'});toast('断电模式已启动');setTimeout(refresh,500);});
document.querySelector('button.z').addEventListener('click',function(){if(!document.getElementById('cfg_lrb'))buildCfg();nav('cfg_panel');loadCfg();});
refresh();setInterval(refresh,2000);
dbg.textContent='V3 Ready';dbg.style.background='#27ae60';
</script>
</body>
</html>
)html";

const char *web_get_page() {
    return g_html_page;
}

} // namespace smarthome
