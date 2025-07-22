#include "UOpenAIClient.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

UOpenAIClient::UOpenAIClient() {}

void UOpenAIClient::SendPromptToGPT(const FString& Prompt)
{
    FString APIKey = TEXT("AIzaSyCazo_SPPHtirsbIOnfUHOBkUEeLwJKPfo");
    FString URL = FString::Printf(TEXT("https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash-preview-05-20:generateContent?key=%s"), *APIKey);
    TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

    TSharedRef<FJsonObject> TextPart = MakeShareable(new FJsonObject);
    TextPart->SetStringField("text", Prompt);

    TArray<TSharedPtr<FJsonValue>> Parts;
    Parts.Add(MakeShareable(new FJsonValueObject(TextPart)));

    TSharedRef<FJsonObject> Content = MakeShareable(new FJsonObject);
    Content->SetArrayField("parts", Parts);

    TArray<TSharedPtr<FJsonValue>> Contents;
    Contents.Add(MakeShareable(new FJsonValueObject(Content)));

    JsonObject->SetArrayField("contents", Contents);

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject, Writer);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(URL);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetContentAsString(RequestBody);
    Request->OnProcessRequestComplete().BindUObject(this, &UOpenAIClient::OnResponseReceived);
    Request->ProcessRequest();
}

void UOpenAIClient::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (!bWasSuccessful || !Response.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Gemini Request Failed"));
        return;
    }
    FString JsonRaw = Response->GetContentAsString();
    UE_LOG(LogTemp, Warning, TEXT("Gemini Response: %s"), *JsonRaw);
    TSharedPtr<FJsonObject> JsonResponse;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonRaw);
    if (FJsonSerializer::Deserialize(Reader, JsonResponse))
    {
        const TArray<TSharedPtr<FJsonValue>>* Candidates;
        if (JsonResponse->TryGetArrayField("candidates", Candidates) && Candidates->Num() > 0)
        {
            const TSharedPtr<FJsonObject> ContentObj = (*Candidates)[0]->AsObject()->GetObjectField("content");
            const TArray<TSharedPtr<FJsonValue>>* Parts;
            if (ContentObj->TryGetArrayField("parts", Parts) && Parts->Num() > 0)
            {
                FString ResultText = (*Parts)[0]->AsObject()->GetStringField("text");
                if (OnT2TResponseReceived.IsBound())
                {
                    OnT2TResponseReceived.Execute(ResultText);
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Gemini Response parts none"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Gemini Response candidates none"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Gemini Response JSON Pathing Failed"));
    }
} 