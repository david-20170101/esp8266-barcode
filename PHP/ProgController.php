<?php
namespace app\prog\controllers;

use fastphp\base\Controller;
use app\prog\models\ProgModel;

class ProgController extends Controller
{

    public function search()
    {
        $raw_input = file_get_contents('php://input');
        $body = json_decode($raw_input, TRUE);
        $prog = array();
        if ( isset($body['MAC']) and isset($body['Index']) and $body['Index']=="255" ) {
            $mySql    = "select sn,reference from [mqtt_4] where qty=1 group by sn,reference ";
            $progModel = new SieModel();
            $progDB    = $progModel->search( $mySql );
            $j = 0;
            if (sizeof($progDB)>0){
                for ($i=0;$i<sizeof($progDB);$i++){
                    $j = $j + pow(2, $progDB[$i]["reference"]-1);
                }
            }
            $prog[0]["Index"] = $j;
            $prog[0]["MAC"] = $body['MAC'];
            $prog[0]["ISOK"] = "1";
        } else if (isset($body['MAC']) and isset($body['Index']) and $body['Index']=="254" and isset($body['CardID']) and $body['CardID']==$body['MAC']) {
            $mySql    = "select sum(QTY) as QTY from [mqtt_3] where SN='" . $body['MAC'] . "' ";
            $progModel = new SieModel();
            $progDB    = $progModel->search( $mySql );
            $prog = $this->false_return($body);
            if (sizeof($progDB)>0){
                $prog[0]["QTY"] = iconv("GBK","UTF-8",$progDB[0]['QTY']);
            } else {
                $prog[0]["QTY"] = "0";
            }
            $prog[0]["ISOK"] = "1";
        } else if (isset($body['MAC']) and isset($body['Index']) and $body['Index']<>"255" and isset($body['CardID']) and $body['CardID']==$body['MAC']) {
            $mySql    = "select * from [mqtt_4] where SN='" . $body['MAC'] . "' and qty=1 and reference=" . $body['Index'] . " ";
			$progModel = new SieModel();
            $progDB    = $progModel->search( $mySql );
            $check = array("Index"=>0,"MAC"=>$body['MAC'],"CardID"=>$body['CardID']);
            if (sizeof($progDB)>0){
                $check["Index"] = pow(2, $progDB[0]['Reference']-1); 
                $mySql  = "select a.SN,b.Sku,b.StyleNo,b.ColorNo,b.SizeNo,b.StyleName,b.ColorName,b.SizeName,b.Unit,sum(a.qty) as Qty  ";
                $mySql .= "from [mqtt_4] a,[mqtt_1] b ";
                $mySql .= "where b.sku=a.sku and a.qty=1 ";
                $mySql .= "and a.reference =" . $body['Index'] . " and a.sn='" . $body['MAC'] . "' ";
                $mySql .= "group by a.sn,b.stylename,b.colorname,b.sizename,b.styleno,b.colorno,b.sizeno,b.unit,b.sku ";
                $progModel = new SieModel();
                $progDB    = $progModel->search( $mySql );
                if (sizeof($progDB)>0){
                    $prog = $this->true_return($check,$progDB);
                }
            } else {
                $prog[0]["Index"] = 0;
                $prog[0]["ISOK"] = "1";
                $prog[0]["MAC"] = $body['MAC'];
            }
        } else if (isset($body['MAC']) and isset($body['CardID']) and $body['CardID']<>$body['MAC'] and isset($body['Index']) and $body['Index']=="254") {   //盘点模式
            $prog = $this->check_stock_by_sn_cradID($body);
        } else if (isset($body['MAC']) and isset($body['CardID']) and $body['CardID']<>$body['MAC'] and isset($body['Index']) and $body['Index']=="253") {   //移仓模式
            $prog = $this->move_stock_by_sn_cradID($body);
        } else if (isset($body['MAC']) and isset($body['CardID']) and $body['CardID']<>$body['MAC'] and isset($body['Index']) and $body['Index']=="0") {     //归仓模式
            $prog = $this->into_stock_by_sn_cradID($body);
        } else if (isset($body['MAC']) and isset($body['CardID']) and $body['CardID']<>$body['MAC']) {
            $prog = $this->outfrom_stock_by_sn_cradID($body);
        }
        $prog = json_encode($prog, true); 
        echo( $prog );
    }
    private function outfrom_stock_by_sn_cradID($outfrom_stock)
    {
        $return = array();
        if ($this->check_mqtt_2_by_cardID($outfrom_stock))
        {
            if ($this->check_mqtt_3_qty_by_sn_cardID($outfrom_stock) and $this->check_mqtt_4_qty_by_sn_index_cardID($outfrom_stock))
            {
                $this->set_mqtt_3_qty_by_sn_cardID($outfrom_stock, 0);
                $this->set_mqtt_4_qty_by_sn_index_cardID($outfrom_stock, 0);
                $mySql = "select * from [mqtt_4] where qty=1 and reference='" . $outfrom_stock['Index'] . "' and sn='" . $outfrom_stock['MAC'] . "' ";
                $progModel = new SieModel();
                $progDB    = $progModel->search( $mySql );
                if (sizeof($progDB)>0)
                {
                    $mySql  = "select a.SN,b.Sku,b.StyleNo,b.ColorNo,b.SizeNo,b.StyleName,b.ColorName,b.SizeName,b.Unit,sum(a.qty) as Qty  ";
                    $mySql .= "from [mqtt_4] a,[mqtt_1] b ";
                    $mySql .= "where b.sku=a.sku and a.qty=1 ";
                    $mySql .= "and a.reference =" . $outfrom_stock['Index'] . " and a.sn='" . $outfrom_stock['MAC'] . "' ";
                    $mySql .= "group by a.sn,b.stylename,b.colorname,b.sizename,b.styleno,b.colorno,b.sizeno,b.unit,b.sku ";
                    $progModel = new SieModel();
                    $progDB    = $progModel->search( $mySql );
                    $outfrom_stock['Index'] = pow(2,$outfrom_stock['Index']-1);
                    $return = $this->true_return($outfrom_stock, $progDB);
                } else {
                    $return = $this->false_return($outfrom_stock);
                    $return[0]['Index'] = "0";
                    $return[0]['ISOK']  = "1";
                }
            } else {
                $outfrom_stock['Index'] = pow(2, $outfrom_stock['Index']-1);
                $return = $this->false_return($outfrom_stock);
            }
        } else {
            $outfrom_stock['Index'] = pow(2, $outfrom_stock['Index']-1);
            $return = $this->false_return($outfrom_stock);
        }
        return $return;
    }
    private function into_stock_by_sn_cradID($into_stock)
    {
        $return = array();
        if ($this->check_mqtt_2_by_cardID($into_stock))
        {
            if ($this->check_mqtt_3_qty_by_sn_cardID($into_stock))
            {
                $return = $this->false_return($into_stock);
            } else {
                $this->set_mqtt_3_qty_by_sn_cardID($into_stock, 1);
                /*****************************/
                $this->add_Head($into_stock);
                /*****************************/
                $return = $this->details_by_cardID($into_stock);
            }
        } else {
            $return = $this->false_return($into_stock);
        }
        return $return;
    }
    private function move_stock_by_sn_cradID($move_stock)
    {
        $return = array();
        if ($this->check_mqtt_2_by_cardID($move_stock))
        {
            if ($this->check_mqtt_3_qty_by_sn_cardID($move_stock))
            {
                $this->set_mqtt_3_qty_by_sn_cardID($move_stock, 0);
                $this->set_mqtt_5_qty_by_cardID($move_stock, 1);
                $return = $this->details_by_cardID($move_stock);
            } else {
                if ($this->check_mqtt_5_qty_by_cardID($move_stock))
                {
                    $this->set_mqtt_5_qty_by_cardID($move_stock, 0);
                    $this->set_mqtt_3_qty_by_sn_cardID($move_stock, 1);
                    $return = $this->details_by_cardID($move_stock);
                } else {
                    $return = $this->false_return($move_stock);
                }
            }
        } else {
            $return = $this->false_return($move_stock);
        }
        return $return;
    }
    private function check_stock_by_sn_cradID($check_stock)
    {
        $return = array();
        if ($this->check_mqtt_2_by_cardID($check_stock))
        {
            if ($this->check_mqtt_3_qty_by_sn_cardID($check_stock))
            {
                $return = $this->details_by_cardID($check_stock);
            } else {
                $return = $this->false_return($check_stock);
            }
        } else {
            $return = $this->false_return($check_stock);
        }
        return $return;
    }
    private function false_return($check)
    {
        $return = array();
        $return[0]["BarCode"]   = " ";
        $return[0]["StyleNo"]   = " ";
        $return[0]["ColorNo"]   = " ";
        $return[0]["SizeNo"]    = " ";
        $return[0]["StyleName"] = " ";
        $return[0]["ColorName"] = " ";
        $return[0]["SizeName"]  = " ";
        $return[0]["Unit"]      = "件";
        $return[0]["QTY"]       = "0";
        $return[0]["MAC"]       = iconv("GBK","UTF-8",$check['MAC']);
        $return[0]["Index"]     = iconv("GBK","UTF-8",$check['Index']);
        $return[0]["CardID"]    = iconv("GBK","UTF-8",$check['CardID']);
        $return[0]["ISOK"]      = "0";
        return $return;
    }
    private function true_return($check,$progDB)
    {
        $return = array();
        $return[0]["BarCode"]   = iconv("GBK","UTF-8",$progDB[0]['Sku']);
        $return[0]["StyleNo"]   = iconv("GBK","UTF-8",$progDB[0]['StyleNo']);
        $return[0]["ColorNo"]   = iconv("GBK","UTF-8",$progDB[0]['ColorNo']);
        $return[0]["SizeNo"]    = iconv("GBK","UTF-8",$progDB[0]['SizeNo']);
        $return[0]["StyleName"] = iconv("GBK","UTF-8",$progDB[0]['StyleName']);
        $return[0]["ColorName"] = iconv("GBK","UTF-8",$progDB[0]['ColorName']);
        $return[0]["SizeName"]  = iconv("GBK","UTF-8",$progDB[0]['SizeName']);
        $return[0]["Unit"]      = iconv("GBK","UTF-8",$progDB[0]['Unit']);
        $return[0]["QTY"]       = iconv("GBK","UTF-8",$progDB[0]['Qty']);
        $return[0]["MAC"]       = iconv("GBK","UTF-8",$check['MAC']);
        $return[0]["Index"]     = iconv("GBK","UTF-8",$check['Index']);
        $return[0]["CardID"]    = iconv("GBK","UTF-8",$check['CardID']);
        $return[0]["ISOK"]      = "1";
        return $return;
    }
    private function details_by_cardID($details)
    {
        $return = array();
        $mySql  = "select a.Sku,a.StyleNo,a.ColorNo,a.SizeNo,a.StyleName,a.ColorName,a.SizeName,a.Unit,'1' as Qty ";
        $mySql .= "from [mqtt_1] a,[mqtt_2] b where a.sku=b.sku and b.cardID='" . $details['CardID'] . "' ";
        $progModel = new SieModel();
        $progDB    = $progModel->search( $mySql );
        if (sizeof($progDB)>0)
        {
            $return = $this->true_return($details,$progDB);
        } else {
            $return = $this->false_return($details);
        }
        return $return;
    }
    private function add_Head($body)
    {
        $Head = '8';
        $check = $body;
        $check['Index'] = $Head;
        if ($this->check_mqtt_2_by_cardID($check))
        {
            $this->set_mqtt_4_qty_by_sn_index_cardID($check, 1);
        }
    }
    private function check_mqtt_2_by_cardID($body)                   //判断卡号是否已绑定
    {
        $mySql    = "select * from [mqtt_2] where cardID='" .$body['CardID']. "' ";
        $progModel = new SieModel();
        $progDB    = $progModel->search( $mySql );
        if (sizeof($progDB)>0){
            return true;
        } else {
            return false;
        }
    }
    private function get_mqtt_2_by_cardID($body)                     //获取卡号绑定信息
    {
        $mySql    = "select * from [mqtt_2] where cardID='" .$body['CardID']. "' ";
        $progModel = new SieModel();
        $progDB    = $progModel->search( $mySql );
        if (sizeof($progDB)>0){
            $return = $progDB[0]['Sku'];
        } else {
            $return = "";
        }
        return $return;
    }
    private function check_mqtt_3_qty_by_sn_cardID($body)            //判断库存数量是否为壹
    {
        $sku      = $this->get_mqtt_2_by_cardID($body);
        $mySql    = "select * from [mqtt_3] where qty=1 and sku='" . $sku . "' and sn='" . $body['MAC'] . "' and cardID='" . $body['CardID'] . "' ";
        $progModel = new SieModel();
        $progDB    = $progModel->search( $mySql );
        if (sizeof($progDB)>0){
            return true;
        } else {
            return false;
        }
    }
    private function set_mqtt_3_qty_by_sn_cardID($body, $qty)              //设置库存数量
    {
        $sku      = $this->get_mqtt_2_by_cardID($body);
        $mySql    = "select * from [mqtt_3] where sku='" . $sku . "' and sn='" . $body['MAC'] . "' and cardID='" . $body['CardID'] . "' ";
        $progModel = new SieModel();
        $progDB    = $progModel->search( $mySql );
        if (sizeof($progDB)>0){
            $mySql    = "update [mqtt_3] set qty=" . $qty . " where sku='" . $sku . "' and sn='" . $body['MAC'] . "' and cardID='" . $body['CardID'] . "' ";
            $progModel = new SieModel();
            $progDB    = $progModel->search( $mySql );
        } else {
            $mySql    = "insert into [mqtt_3] (sn,cardID,sku,qty) values ('" . $body['MAC'] . "','" . $body['CardID'] . "','" . $sku . "'," . $qty . ")"; 
            $progModel = new SieModel();
            $progDB    = $progModel->search( $mySql );
        }
    }
    private function check_mqtt_4_qty_by_sn_index_cardID($body)               //判断配货单据数量是否为壹
    {
        $sku      = $this->get_mqtt_2_by_cardID($body);
        $mySql    = "select * from [mqtt_4] where qty=1 and reference='" . $body['Index'] . "' and sku='" . $sku . "' and sn='" . $body['MAC'] . "' and cardID='" . $body['CardID'] . "' ";
        $progModel = new SieModel();
        $progDB    = $progModel->search( $mySql );
        if (sizeof($progDB)>0){
            return true;
        } else {
            return false;
        }
    }
    private function set_mqtt_4_qty_by_sn_index_cardID($body, $qty)              //设置配货单据数量
    {
        $sku      = $this->get_mqtt_2_by_cardID($body);
        $mySql    = "select * from [mqtt_4] where reference='" . $body['Index'] . "' and sku='" .$sku. "' and sn='" . $body['MAC'] . "' and cardID='" . $body['CardID'] . "' ";
        $progModel = new SieModel();
        $progDB    = $progModel->search( $mySql );
        if (sizeof($progDB)>0){
            $mySql    = "update [mqtt_4] set qty=" . $qty . " where reference='" . $body['Index'] . "' and sku='" . $sku . "' and sn='" . $body['MAC'] . "' and cardID='" . $body['CardID'] . "' ";
            $progModel = new SieModel();
            $progDB    = $progModel->search( $mySql );
        } else {
            $mySql    = "insert into [mqtt_4] (sn,cardID,sku,qty,reference) values ('" . $body['MAC'] . "','" . $body['CardID'] . "','" . $sku . "'," . $qty . ",'" . $body['Index'] . "')"; 
            $progModel = new SieModel();
            $progDB    = $progModel->search( $mySql );
        }
    }
    private function check_mqtt_5_qty_by_cardID($body)               //判断临时库存数量是否为壹
    {
        $mySql    = "select * from [mqtt_5] where qty=1 and cardID='" . $body['CardID'] . "' ";
        $progModel = new SieModel();
        $progDB    = $progModel->search( $mySql );
        if (sizeof($progDB)>0){
            return true;
        } else {
            return false;
        }
    }
    private function set_mqtt_5_qty_by_cardID($body, $qty)              //设置库存数量
    {
        $sku      = $this->get_mqtt_2_by_cardID($body);
        $mySql    = "select * from [mqtt_5] where sku='" .$sku. "' and cardID='" . $body['CardID'] . "' ";
        $progModel = new SieModel();
        $progDB    = $progModel->search( $mySql );
        if (sizeof($progDB)>0){
            $mySql    = "update [mqtt_5] set qty=" . $qty . " where sku='" . $sku . "' and cardID='" . $body['CardID'] . "' ";
            $progModel = new SieModel();
            $progDB    = $progModel->search( $mySql );
        } else {
            $mySql    = "insert into [mqtt_5] (cardID,sku,qty) values ('" . $body['CardID'] . "','" . $sku . "'," . $qty .")"; 
            $progModel = new SieModel();
            $progDB    = $progModel->search( $mySql );
        }
    }
}
