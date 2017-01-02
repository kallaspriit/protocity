module Api.NfcTag exposing (..)

import Http
import Json.Decode exposing (Decoder, float, int, string, bool)
import Json.Decode.Pipeline exposing (decode, hardcoded, optional, required, requiredAt)
import Model.Msg exposing (..)
import Model.NfcTag exposing (..)
import BasicAuth exposing (..)


loadNfcTag : Int -> Cmd Msg
loadNfcTag id =
    let
        url =
            "https://telia.cumulocity.com/inventory/managedObjects/" ++ (toString id)

        request =
            Http.request
                { method = "GET"
                , headers =
                    -- TODO move the credentials to a configuration file
                    [ buildAuthorizationHeader "priit.kallas@telia.ee" "purgisupp" ]
                , url = url
                , body = Http.emptyBody
                , expect = Http.expectJson nfcTagDecoder
                , timeout = Nothing
                , withCredentials = False
                }
    in
        Http.send NfcTagResult request


nfcTagDecoder : Decoder NfcTag
nfcTagDecoder =
    decode NfcTag
        |> requiredAt [ "id" ] string
        |> requiredAt [ "com_stagnationlab_c8y_driver_fragments_TagSensor", "tagName" ] string
        |> requiredAt [ "com_stagnationlab_c8y_driver_fragments_TagSensor", "tagActive" ] bool
