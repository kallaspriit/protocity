module Api.User exposing (..)

import Html exposing (..)
import Http
import Task
import Json.Decode exposing (Decoder, float, int, string)
import Json.Decode.Pipeline exposing (decode, hardcoded, optional, required, requiredAt)
import Model.Msg exposing (..)
import Model.User exposing (..)


loadUser : Int -> Cmd Msg
loadUser id =
    let
        url =
            "https://reqres.in/api/users/" ++ (toString id) ++ "?delay=1"

        request =
            Http.get url userDecoder
    in
        Http.send UserResult request



-- http json response decoders for a response like
-- {"data":{"id":1,"first_name":"george","last_name":"bluth","avatar":"https://s3.amazonaws.com/uifaces/faces/twitter/calebogden/128.jpg"}}


userDecoder : Decoder User
userDecoder =
    decode User
        |> requiredAt [ "data", "id" ] int
        |> requiredAt [ "data", "first_name" ] string
        |> requiredAt [ "data", "last_name" ] string
        |> requiredAt [ "data", "avatar" ] string
