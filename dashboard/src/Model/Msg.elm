module Model.Msg exposing (..)

import Http
import Model.User exposing (..)
import Model.NfcTag exposing (..)


type Msg
    = LoadUser
    | UserResult (Result Http.Error User)
    | LoadNfcTag
    | NfcTagResult (Result Http.Error NfcTag)
